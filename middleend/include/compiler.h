#ifndef ry_compiler_h
#define ry_compiler_h

#include "chunk.h"
#include "expr.h"
#include "stmt.h"

namespace RyRuntime {

	struct Local {
		Backend::Token name;
		int depth;
		bool isCaptured = false;
	};

	class Compiler : public Backend::ExprVisitor, public Backend::StmtVisitor {
	public:
		// Main entry point: takes source and returns a compiled chunk
		bool compile(const std::vector<std::shared_ptr<Backend::Stmt>> &statements, Chunk *chunk);

	private:
		void visitMath(Backend::MathExpr &expr);
		void visitGroup(Backend::GroupExpr &expr);
		void visitVariable(Backend::VariableExpr &expr);
		void visitValue(Backend::ValueExpr &expr);
		void visitLogical(Backend::LogicalExpr &expr);
		void visitAssign(Backend::AssignExpr &expr);
		void visitCall(Backend::CallExpr &expr);
		void visitThis(Backend::ThisExpr &expr);
		void visitGet(Backend::GetExpr &expr);
		void visitMap(Backend::MapExpr &expr);
		void visitRange(Backend::RangeExpr &expr);
		void visitSet(Backend::SetExpr &expr);
		void visitIndexSet(Backend::IndexSetExpr &expr);
		void visitIndex(Backend::IndexExpr &expr);
		void visitList(Backend::ListExpr &expr);
		void visitBitwiseOr(Backend::BitwiseOrExpr &expr);
		void visitBitwiseXor(Backend::BitwiseXorExpr &expr);
		void visitBitwiseAnd(Backend::BitwiseAndExpr &expr);
		void visitPrefix(Backend::PrefixExpr &expr);
		void visitPostfix(Backend::PostfixExpr &expr);
		void visitShift(Backend::ShiftExpr &expr);
		void visitExpressionStmt(Backend::ExpressionStmt &stmt);
		void visitStopStmt(Backend::StopStmt &stmt);
		void visitSkipStmt(Backend::SkipStmt &stmt);
		void visitImportStmt(Backend::ImportStmt &stmt);
		void visitAliasStmt(Backend::AliasStmt &stmt);
		void visitNamespaceStmt(Backend::NamespaceStmt &stmt);
		void visitEachStmt(Backend::EachStmt &stmt);
		void visitBlockStmt(Backend::BlockStmt &stmt);
		void visitReturnStmt(Backend::ReturnStmt &stmt);
		void visitForStmt(Backend::ForStmt &stmt);
		void visitAttemptStmt(Backend::AttemptStmt &stmt);
		void visitPanicStmt(Backend::PanicStmt &stmt);
		void visitIfStmt(Backend::IfStmt &stmt);
		void visitWhileStmt(Backend::WhileStmt &stmt);
		void visitClassStmt(Backend::ClassStmt &stmt);
		void visitFunctionStmt(Backend::FunctionStmt &stmt);
		void visitVarStmt(Backend::VarStmt &stmt);

		// Helper to write opcodes to the chunk
		void emitByte(uint8_t byte);
		void emitBytes(uint8_t byte1, uint8_t byte2);
		void emitConstant(RyValue value);
		int makeConstant(RyValue value);

		// Jump helpers
		int emitJump(uint8_t instruction);
		void patchJump(int offset);
		void emitLoop(int loopStart);

		Chunk *compilingChunk;
		void compileStatement(std::shared_ptr<Backend::Stmt> stmt);
		void compileExpression(std::shared_ptr<Backend::Expr> expr);

		// Scope & Locals
		std::vector<Local> locals;
		int scopeDepth = 0;
		void beginScope();
		void endScope();
		int resolveLocal(Backend::Token &name);
		void addLocal(Backend::Token name);
	};
} // namespace RyRuntime

#endif
