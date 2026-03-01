#include "compiler.h"
#include <iostream>
#include <vector>
#include "chunk.h"
#include "class.h"
#include "func.h"
#include "stmt.h"
#include "token.h"
#include "tools.h"

using namespace Backend;

namespace RyRuntime {
	bool Compiler::compile(const std::vector<std::shared_ptr<Backend::Stmt>> &statements, Chunk *chunk) {
		this->compilingChunk = chunk;
		this->locals.clear();
		this->scopeDepth = 0;
		Token internal;
		internal.lexeme = "(script)";
		addLocal(internal);

		for (const auto &stmt: statements) {
			compileStatement(stmt);
		}

		emitByte(OP_RETURN);
		return true; // Return false if there's a compilation error
	}

	void Compiler::compileStatement(std::shared_ptr<Backend::Stmt> stmt) {
		if (stmt)
			stmt->accept(*this);
	}

	void Compiler::compileExpression(std::shared_ptr<Backend::Expr> expr) {
		if (expr)
			expr->accept(*this);
	}
	void Compiler::compileMethod(std::shared_ptr<Backend::FunctionStmt> stmt) {
		track(stmt->name);

		Compiler subCompiler(this, this->sourceCode);
		subCompiler.currentClass = this->currentClass;
		auto function = std::make_shared<Frontend::RyFunction>();
		function->name = stmt->name.lexeme;
		function->arity = stmt->parameters.size();

		subCompiler.compilingChunk = &function->chunk;
		subCompiler.beginScope();

		// Slot 0 is "this" for methods!
		Token thisToken;
		thisToken.lexeme = "this";
		subCompiler.addLocal(thisToken);

		for (const auto &param: stmt->parameters) {
			subCompiler.addLocal(param.name);
		}

		for (const auto &bodyStmt: stmt->body) {
			subCompiler.compileStatement(bodyStmt);
		}

		subCompiler.emitByte(OP_NULL);
		subCompiler.emitByte(OP_RETURN);
		subCompiler.endScope();

		emitBytes(OP_CLOSURE, (uint8_t) makeConstant(RyValue(function)));

		// Emit upvalue data
		for (int i = 0; i < subCompiler.upvalues.size(); i++) {
			emitByte(subCompiler.upvalues[i].isLocal ? 1 : 0);
			emitByte(subCompiler.upvalues[i].index);
		}
	}

	// --- Bytecode Helpers ---

	void Compiler::emitByte(uint8_t byte) { compilingChunk->write(byte, currentLine, currentColumn); }

	void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
		emitByte(byte1);
		emitByte(byte2);
	}

	void Compiler::emitConstant(RyValue value) { emitBytes(OP_CONSTANT, (uint8_t) makeConstant(value)); }

	int Compiler::makeConstant(RyValue value) {
		int constant = compilingChunk->addConstant(value);
		if (constant > 255) {
			std::cerr << "Too many constants in one chunk!" << std::endl;
			return 0;
		}
		return constant;
	}

	int Compiler::emitJump(uint8_t instruction) {
		emitByte(instruction);
		emitByte(0xff);
		emitByte(0xff);
		return compilingChunk->code.size() - 2;
	}

	void Compiler::patchJump(int offset) {
		// -2 to adjust for the jump offset itself
		int jump = compilingChunk->code.size() - offset - 2;

		if (jump > UINT16_MAX) {
			std::cerr << "Too much code to jump over!" << std::endl;
		}

		compilingChunk->code[offset] = (jump >> 8) & 0xff;
		compilingChunk->code[offset + 1] = jump & 0xff;
	}

	void Compiler::emitLoop(int loopStart) {
		emitByte(OP_LOOP);

		int offset = compilingChunk->code.size() - loopStart + 2;
		if (offset > UINT16_MAX)
			std::cerr << "Loop body too large!" << std::endl;

		emitByte((offset >> 8) & 0xff);
		emitByte(offset & 0xff);
	}

	// --- Scope Helpers ---

	void Compiler::beginScope() { scopeDepth++; }

	void Compiler::endScope() {
		scopeDepth--;
		// Pop locals that were in this scope
		while (!locals.empty() && locals.back().depth > scopeDepth) {
			emitByte(OP_POP);
			locals.pop_back();
		}
	}

	void Compiler::addLocal(Token name) {
		Local local = Local(name, scopeDepth, false);
		locals.push_back(local);
	}

	int Compiler::resolveLocal(Token &name) {
		for (int i = locals.size() - 1; i >= 0; i--) {
			Local &local = locals[i];
			if (name.lexeme == local.name.lexeme) {
				return i;
			}
		}
		return -1;
	}
	int Compiler::resolveUpvalue(Token &name) {
		if (enclosing == nullptr)
			return -1; // We hit the top (global scope)

		int local = enclosing->resolveLocal(name);
		if (local != -1) {
			return addUpvalue((uint8_t) local, true);
		}

		int upvalue = enclosing->resolveUpvalue(name);
		if (upvalue != -1) {
			return addUpvalue((uint8_t) upvalue, false);
		}

		return -1;
	}

	int Compiler::addUpvalue(uint8_t index, bool isLocal) {
		for (int i = 0; i < upvalues.size(); i++) {
			Upvalue &upvalue = upvalues[i];
			if (upvalue.index == index && upvalue.isLocal == isLocal) {
				return i;
			}
		}

		if (upvalues.size() == 256) {
			RyTools::report(currentLine, currentColumn, "", "Too many closure variables in function.", sourceCode);
			RyTools::hadError = true;
			return 0;
		}

		Upvalue upvalue;
		upvalue.isLocal = isLocal;
		upvalue.index = index;
		upvalues.push_back(upvalue);
		return (int) upvalues.size() - 1;
	}

	// --- Error reporting ---
	void Compiler::error(const Backend::Token &token, const std::string &message) {
		RyTools::report(token.line, token.column, "", message, this->sourceCode);

		RyTools::hadError = true;
	}

	void Compiler::track(Token token) {
		this->currentLine = token.line;
		this->currentColumn = token.column;
	}

	// --- Visitors ---

	void Compiler::visitMath(MathExpr &expr) {
		track(expr.op_t);

		compileExpression(expr.left);
		compileExpression(expr.right);

		switch (expr.op_t.type) {
			case Backend::TokenType::PLUS:
				emitByte(OP_ADD);
				break;
			case Backend::TokenType::MINUS:
				emitByte(OP_SUBTRACT);
				break;
			case TokenType::STAR:
				emitByte(OP_MULTIPLY);
				break;
			case TokenType::DIVIDE:
				emitByte(OP_DIVIDE);
				break;
			case TokenType::PERCENT:
				emitByte(OP_MODULO);
				break;
			case TokenType::EQUAL_EQUAL:
				emitByte(OP_EQUAL);
				break;
			case TokenType::BANG_EQUAL:
				emitBytes(OP_EQUAL, OP_NOT);
				break;
			case TokenType::GREATER:
				emitByte(OP_GREATER);
				break;
			case TokenType::GREATER_EQUAL:
				emitBytes(OP_LESS, OP_NOT);
				break;
			case TokenType::LESS:
				emitByte(OP_LESS);
				break;
			case TokenType::LESS_EQUAL:
				emitBytes(OP_GREATER, OP_NOT);
				break;
			default:
				break;
		}
	}

	void Compiler::visitGroup(GroupExpr &expr) { compileExpression(expr.expression); }

	void Compiler::visitVariable(VariableExpr &expr) {
		track(expr.name);
		std::string name = expr.name.lexeme;

		int arg = resolveLocal(expr.name);
		if (arg != -1) {
			emitBytes(OP_GET_LOCAL, (uint8_t) arg);
			return;
		}

		arg = resolveUpvalue(expr.name);
		if (arg != -1) {
			emitBytes(OP_GET_UPVALUE, (uint8_t) arg);
			return;
		}

		if (name.find("::") != std::string::npos) {
			emitBytes(OP_GET_GLOBAL, (uint8_t) makeConstant(RyValue(name)));
			return;
		}

		bool isNative = nativeNames.count(name) > 0;

		if (!currentNamespace.empty() && !isNative && !name.starts_with("native") && name.find("::") == std::string::npos) {
			name = currentNamespace + "::" + name;
		}

		emitBytes(OP_GET_GLOBAL, (uint8_t) makeConstant(RyValue(name)));
	}

	void Compiler::visitValue(ValueExpr &expr) {
		track(expr.value);

		if (expr.value.type == TokenType::TRUE) {
			emitByte(OP_TRUE);
		} else if (expr.value.type == TokenType::FALSE) {
			emitByte(OP_FALSE);
		} else if (expr.value.type == TokenType::NULL_TOKEN) {
			emitByte(OP_NULL);
		} else if (expr.value.type == TokenType::NUMBER) {
			double val = std::stod(expr.value.lexeme);
			emitConstant(RyValue(val));
		} else if (expr.value.type == TokenType::STRING) {
			emitConstant(RyValue(expr.value.lexeme));
		}
	}

	void Compiler::visitLogical(LogicalExpr &expr) {
		track(expr.op_t);

		compileExpression(expr.left);
		if (expr.op_t.type == TokenType::AND) {
			int endJump = emitJump(OP_JUMP_IF_FALSE);
			emitByte(OP_POP);
			compileExpression(expr.right);
			patchJump(endJump);
		} else { // OR
			int elseJump = emitJump(OP_JUMP_IF_FALSE);
			int endJump = emitJump(OP_JUMP);
			patchJump(elseJump);
			emitByte(OP_POP);
			compileExpression(expr.right);
			patchJump(endJump);
		}
	}
	void Compiler::visitRange(RangeExpr &expr) {
		track(expr.op_t);

		// Compile the start (e.g., 1)
		compileExpression(expr.leftBound);
		// Compile the end (e.g., 10)
		compileExpression(expr.rightBound);

		// Ry Specialty: Tell the VM to build a list from this range
		emitByte(OP_BUILD_RANGE_LIST);
	}
	void Compiler::visitList(ListExpr &expr) {
		for (const auto &element: expr.elements) {
			compileExpression(element);
		}
		// Emit an instruction that knows how many elements to grab from the stack
		emitBytes(OP_BUILD_LIST, (uint8_t) expr.elements.size());
	}

	void Compiler::visitAssign(AssignExpr &expr) {
		track(expr.name);
		compileExpression(expr.value);
		int arg = resolveLocal(expr.name);
		if (arg != -1) {
			emitBytes(OP_SET_LOCAL, (uint8_t) arg);
			return;
		}
		arg = resolveUpvalue(expr.name);
		if (arg != -1) {
			emitBytes(OP_SET_UPVALUE, (uint8_t) arg);
			return;
		}

		if (expr.name.lexeme.find("::") != std::string::npos) {
			emitBytes(OP_SET_GLOBAL, (uint8_t) makeConstant(RyValue(expr.name.lexeme)));
		} else {
			std::string name = expr.name.lexeme;

			if (name.find("::") == std::string::npos && !currentNamespace.empty()) {
				name = currentNamespace + "::" + name;
			}

			emitBytes(OP_SET_GLOBAL, (uint8_t) makeConstant(RyValue(name)));
		}
	}

	void Compiler::visitCall(CallExpr &expr) {
		track(expr.Paren);
		compileExpression(expr.callee);
		for (const auto &arg: expr.arguments) {
			compileExpression(arg);
		}
		emitBytes(OP_CALL, (uint8_t) expr.arguments.size());
	}

	void Compiler::visitExpressionStmt(ExpressionStmt &stmt) {
		compileExpression(stmt.expression);
		if (std::dynamic_pointer_cast<AssignExpr>(stmt.expression) ||
				std::dynamic_pointer_cast<IndexSetExpr>(stmt.expression)) {
			return;
		}
		emitByte(OP_POP);
	}

	void Compiler::visitBlockStmt(BlockStmt &stmt) {
		beginScope();
		for (const auto &s: stmt.statements) {
			compileStatement(s);
		}
		endScope();
	}

	void Compiler::visitIfStmt(IfStmt &stmt) {
		compileExpression(stmt.condition);
		int thenJump = emitJump(OP_JUMP_IF_FALSE);
		emitByte(OP_POP);

		compileStatement(stmt.thenBranch);

		int elseJump = emitJump(OP_JUMP);
		patchJump(thenJump);
		emitByte(OP_POP);

		if (stmt.elseBranch) {
			compileStatement(stmt.elseBranch);
		}
		patchJump(elseJump);
	}

	void Compiler::visitWhileStmt(WhileStmt &stmt) {
		int loopStart = compilingChunk->code.size();


		LoopContext context = LoopContext();
		context.startIP = loopStart;
		context.scopeDepth = this->scopeDepth;
		context.type = LOOP_WHILE;
		loopStack.push_back(context);

		compileExpression(stmt.condition);


		int exitJump = emitJump(OP_JUMP_IF_FALSE);
		emitByte(OP_POP);

		compileStatement(stmt.body);
		emitLoop(loopStart);

		patchJump(exitJump);
		emitByte(OP_POP);
		for (int location: context.breakJumps) {
			patchJump(location);
		}
		loopStack.pop_back();
	}

	void Compiler::visitForStmt(ForStmt &stmt) {
		beginScope();
		if (stmt.init)
			compileStatement(stmt.init);

		int loopStart = compilingChunk->code.size();
		LoopContext context = LoopContext();
		context.startIP = loopStart;
		context.scopeDepth = this->scopeDepth;
		context.type = LOOP_FOR;
		loopStack.push_back(context);

		int exitJump = -1;
		if (stmt.condition) {
			compileExpression(stmt.condition);
			exitJump = emitJump(OP_JUMP_IF_FALSE);
			emitByte(OP_POP);
		}

		compileStatement(stmt.body);

		if (stmt.increment) {
			compileExpression(stmt.increment);
			emitByte(OP_POP);
		}

		emitLoop(loopStart);

		if (exitJump != -1) {
			patchJump(exitJump);
			emitByte(OP_POP);
		}

		for (int location: context.breakJumps) {
			patchJump(location);
		}
		loopStack.pop_back();
		endScope();
	}

	void Compiler::visitVarStmt(VarStmt &stmt) {
		track(stmt.name);
		if (stmt.initializer) {
			compileExpression(stmt.initializer);
		} else {
			emitByte(OP_NULL);
		}

		if (scopeDepth > 0) {
			std::string name = stmt.name.lexeme;
			size_t lastColon = name.find_last_of(':');
			if (lastColon != std::string::npos) {
				Token baseName = stmt.name;
				baseName.lexeme = name.substr(lastColon + 1);
				addLocal(baseName);
			} else {
				addLocal(stmt.name);
			}
		} else {
			std::string name = stmt.name.lexeme;
			emitBytes(OP_DEFINE_GLOBAL, (uint8_t) makeConstant(RyValue(name)));
		}
	}

	void Compiler::visitReturnStmt(ReturnStmt &stmt) {
		track(stmt.keyword);
		if (stmt.value)
			compileExpression(stmt.value);
		else
			emitByte(OP_NULL);
		emitByte(OP_RETURN);
	}

	void Compiler::visitPrefix(PrefixExpr &expr) {
		track(expr.prefix);
		compileExpression(expr.right);
		if (expr.prefix.type == TokenType::MINUS)
			emitByte(OP_NEGATE);
		else if (expr.prefix.type == TokenType::BANG)
			emitByte(OP_NOT);
	}

	void Compiler::visitPanicStmt(PanicStmt &stmt) {
		track(stmt.keyword);
		if (stmt.message)
			compileExpression(stmt.message);
		else
			emitByte(OP_NULL);
		emitByte(OP_PANIC);
	}

	void Compiler::visitClassStmt(ClassStmt &stmt) {
		track(stmt.name);

		auto classCompiler = std::make_shared<Frontend::ClassCompiler>();
		classCompiler->enclosing = currentClass;
		currentClass = classCompiler;

		uint8_t nameConst = (uint8_t) makeConstant(RyValue(stmt.name.lexeme));
		emitBytes(OP_CLASS, nameConst);
		emitBytes(OP_DEFINE_GLOBAL, nameConst);

		emitBytes(OP_GET_GLOBAL, nameConst);

		if (stmt.superclass != nullptr) {
			compileExpression(stmt.superclass);

			emitByte(OP_INHERIT);
		}


		for (const auto &method: stmt.methods) {
			compileMethod(method);

			uint8_t methodConst = (uint8_t) makeConstant(RyValue(method->name.lexeme));
			emitBytes(OP_METHOD, methodConst);
		}

		currentClass = currentClass->enclosing;
		emitByte(OP_POP);
	}

	void Compiler::visitThis(ThisExpr &expr) {
		if (currentClass == nullptr) {
			error(expr.keyword, "Cannot use 'this' outside of a class.");
			return;
		}
		track(expr.keyword);
		emitBytes(OP_GET_LOCAL, 0);
	}
	void Compiler::visitGet(GetExpr &expr) {
		track(expr.name);
		compileExpression(expr.object);
		emitBytes(OP_GET_PROPERTY, (uint8_t) makeConstant(RyValue(expr.name.lexeme)));
	}
	void Compiler::visitSet(SetExpr &expr) {
		track(expr.name);
		compileExpression(expr.object);
		compileExpression(expr.value);
		emitBytes(OP_SET_PROPERTY, (uint8_t) makeConstant(RyValue(expr.name.lexeme)));
	}
	void Compiler::visitFunctionStmt(FunctionStmt &stmt) {
		track(stmt.name);

		Compiler subCompiler(this, this->sourceCode);

		auto function = std::make_shared<Frontend::RyFunction>();
		function->name = stmt.name.lexeme;
		function->arity = stmt.parameters.size();

		subCompiler.compilingChunk = &function->chunk;

		subCompiler.beginScope();
		subCompiler.addLocal(Token());

		for (const auto &param: stmt.parameters) {
			subCompiler.addLocal(param.name);
		}

		for (const auto &bodyStmt: stmt.body) {
			subCompiler.compileStatement(bodyStmt);
		}

		subCompiler.emitByte(OP_NULL);
		subCompiler.emitByte(OP_RETURN);
		subCompiler.endScope();

		emitBytes(OP_CLOSURE, (uint8_t) makeConstant(RyValue(function)));
		function->upvalueCount = subCompiler.upvalues.size();

		for (int i = 0; i < subCompiler.upvalues.size(); i++) {
			emitByte(subCompiler.upvalues[i].isLocal ? 1 : 0);
			emitByte(subCompiler.upvalues[i].index);
		}

		emitBytes(OP_DEFINE_GLOBAL, (uint8_t) makeConstant(RyValue(stmt.name.lexeme)));
	}
	void Compiler::visitMap(MapExpr &expr) {
		track(expr.braceToken);

		for (const auto &item: expr.items) {
			// Compile the Key
			compileExpression(item.first);
			// Compile the Value
			compileExpression(item.second);
		}

		// Emit the instruction with the number of pairs to collect
		emitBytes(OP_BUILD_MAP, (uint8_t) expr.items.size());
	}
	void Compiler::visitIndexSet(IndexSetExpr &expr) {
		track(expr.bracket);
		compileExpression(expr.object);
		compileExpression(expr.index);
		compileExpression(expr.value);
		emitByte(OP_SET_INDEX);
	}
	void Compiler::visitIndex(IndexExpr &expr) {
		track(expr.bracket);
		compileExpression(expr.object);
		compileExpression(expr.index);
		emitByte(OP_GET_INDEX);
	}
	void Compiler::visitBitwiseOr(BitwiseOrExpr &expr) {
		track(expr.op_t);
		compileExpression(expr.left);
		compileExpression(expr.right);
		emitByte(OP_BITWISE_OR);
	}
	void Compiler::visitBitwiseXor(BitwiseXorExpr &expr) {
		track(expr.op_t);
		compileExpression(expr.left);
		compileExpression(expr.right);
		emitByte(OP_BITWISE_XOR);
	}
	void Compiler::visitBitwiseAnd(BitwiseAndExpr &expr) {
		track(expr.op_t);
		compileExpression(expr.left);
		compileExpression(expr.right);
		emitByte(OP_BITWISE_AND);
	}
	void Compiler::visitPostfix(PostfixExpr &expr) {
		track(expr.postfix);

		// Try to see if the left side is a variable
		// Cast the 'left' Expr to a VariableExpr to get the name
		auto var = std::dynamic_pointer_cast<VariableExpr>(expr.left);

		if (var) {
			// Get the current value onto the stack
			int arg = resolveLocal(var->name);
			if (arg != -1) {
				emitBytes(OP_GET_LOCAL, (uint8_t) arg);
			} else {
				emitBytes(OP_GET_GLOBAL, (uint8_t) makeConstant(RyValue(var->name.lexeme)));
			}

			// Copy the value
			emitByte(OP_COPY);

			// Push the increment value
			emitConstant(RyValue(1.0));

			// Add or Subtract
			if (expr.postfix.type == TokenType::PLUS_PLUS) {
				emitByte(OP_ADD);
			} else {
				emitByte(OP_SUBTRACT);
			}

			// Store the NEW value back into the variable
			if (arg != -1) {
				emitBytes(OP_SET_LOCAL, (uint8_t) arg);
			} else {
				emitBytes(OP_SET_GLOBAL, (uint8_t) makeConstant(RyValue(var->name.lexeme)));
			}

		} else {
		}
	}
	void Compiler::visitShift(ShiftExpr &expr) {
		track(expr.op_t);
		compileExpression(expr.left);
		compileExpression(expr.right);
		if (expr.op_t.type == TokenType::LESS_LESS) {
			emitByte(OP_LEFT_SHIFT);
		} else {
			emitByte(OP_RIGHT_SHIFT);
		}
	}
	void Compiler::visitStopStmt(StopStmt &stmt) {
		track(stmt.keyword);
		if (loopStack.empty()) {
			error(stmt.keyword, "Cannot use 'stop' outside of a loop.");
			return;
		}

		int count = 0;
		for (int i = locals.size() - 1; i >= 0; i--) {
			if (locals[i].depth > loopStack.back().scopeDepth) {
				count++;
			} else {
				break;
			}
		}
		for (int i = 0; i < count; i++)
			emitByte(OP_POP);

		if (loopStack.back().type == LOOP_EACH) {
			emitBytes(OP_POP, OP_POP);
		}

		emitByte(OP_JUMP);
		emitByte(0xff);
		emitByte(0xff);
		loopStack.back().breakJumps.push_back(compilingChunk->code.size() - 2);
	}
	void Compiler::visitSkipStmt(SkipStmt &stmt) {
		track(stmt.keyword);
		if (loopStack.empty()) {
			error(stmt.keyword, "Cannot use 'skip' outside of a loop.");
			return;
		}

		int localsToPop = 0;
		for (int i = locals.size() - 1; i >= 0; i--) {
			if (locals[i].depth > loopStack.back().scopeDepth) {
				localsToPop++;
			} else {
				break;
			}
		}

		for (int i = 0; i < localsToPop; i++) {
			emitByte(OP_POP);
		}

		emitLoop(loopStack.back().startIP);
	}
	void Compiler::visitImportStmt(ImportStmt &stmt) {
		stmt.module->accept(*this);
		emitByte(OP_IMPORT);
		emitByte(OP_POP);
	}
	void Compiler::visitAliasStmt(AliasStmt &stmt) {
		track(stmt.name);
		// Evaluate the expression we are aliasing (e.g., Math.sqrt)
		compileExpression(stmt.aliasExpr);

		// Define it in the global map under the NEW name
		uint8_t constant = (uint8_t) makeConstant(RyValue(stmt.name.lexeme));
		emitBytes(OP_DEFINE_GLOBAL, constant);
	}
	void Compiler::visitNamespaceStmt(NamespaceStmt &stmt) {
		track(stmt.name);
		std::string lastNamespace = currentNamespace;
		currentNamespace = stmt.name.lexeme;
		// compile the body
		for (const auto &s: stmt.body) {
			compileStatement(s);
		}
		currentNamespace = lastNamespace;
	}
	void Compiler::visitEachStmt(EachStmt &stmt) {
		track(stmt.id);
		compileExpression(stmt.collection);
		emitConstant(RyValue(0.0));

		beginScope();
		Token dummy;
		addLocal(dummy); // Collection
		addLocal(dummy); // Index

		int loopStart = compilingChunk->code.size();

		LoopContext context = LoopContext();
		context.startIP = loopStart;
		context.scopeDepth = this->scopeDepth;
		context.type = LOOP_EACH;
		loopStack.push_back(context);

		int exitJump = emitJump(OP_FOR_EACH_NEXT);

		beginScope();
		addLocal(stmt.id); // Register variable name at a higher scope depth

		compileStatement(stmt.body);

		endScope(); // This automatically emits OP_POP for variable name and cleans locals

		emitLoop(loopStart);
		patchJump(exitJump);

		endScope(); // This emits OP_POP, OP_POP for the Index and Collection


		for (int location: loopStack.back().breakJumps) {
			patchJump(location);
		}
		loopStack.pop_back();
	}
	void Compiler::visitAttemptStmt(AttemptStmt &stmt) {
		// Emit OP_ATTEMPT and a placeholder for the jump to the 'fail' block
		int jumpToFail = emitJump(OP_ATTEMPT);

		// Compile the 'attempt' body
		beginScope();
		for (const auto &s: stmt.attemptBody) {
			compileStatement(s);
		}
		endScope();

		// If we get here, no panic happened. Remove the safety net.
		emitByte(OP_END_ATTEMPT);

		// Jump over the 'fail' block
		int skipFail = emitJump(OP_JUMP);

		// Patch the OP_ATTEMPT jump so it lands HERE if a panic occurs
		patchJump(jumpToFail);

		// Handle the error variable
		beginScope();
		addLocal(stmt.error); // The VM pushes the error message; track it here

		for (const auto &s: stmt.failBody) {
			compileStatement(s);
		}

		endScope(); // Pops the error variable

		// Patch the skipFail jump so the 'attempt' block finishes here
		patchJump(skipFail);
	}
} // namespace RyRuntime
