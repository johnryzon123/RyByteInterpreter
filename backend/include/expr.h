#pragma once
#include <memory>
#include <utility>
#include <vector>
#include "token.h"

namespace Backend {
	struct ExprVisitor;

	struct Expr {
		virtual ~Expr() = default;
		virtual void accept(ExprVisitor &visitor) = 0;
	};
	struct ExprVisitor {
		virtual ~ExprVisitor() = default;
		virtual void visitValue(struct ValueExpr &expr) = 0;
		virtual void visitMath(struct MathExpr &expr) = 0;
		virtual void visitPrefix(struct PrefixExpr &expr) = 0;
		virtual void visitGroup(struct GroupExpr &expr) = 0;
		virtual void visitVariable(struct VariableExpr &expr) = 0;
		virtual void visitCall(struct CallExpr &expr) = 0;
		virtual void visitLogical(struct LogicalExpr &expr) = 0;
		virtual void visitAssign(struct AssignExpr &expr) = 0;
		virtual void visitList(struct ListExpr &expr) = 0;
		virtual void visitIndex(struct IndexExpr &expr) = 0;
		virtual void visitGet(struct GetExpr &expr) = 0;
		virtual void visitSet(struct SetExpr &expr) = 0;
		virtual void visitMap(struct MapExpr &expr) = 0;
		virtual void visitIndexSet(struct IndexSetExpr &expr) = 0;
		virtual void visitRange(struct RangeExpr &expr) = 0;
		virtual void visitBitwiseAnd(struct BitwiseAndExpr &expr) = 0;
		virtual void visitBitwiseOr(struct BitwiseOrExpr &expr) = 0;
		virtual void visitBitwiseXor(struct BitwiseXorExpr &expr) = 0;
		virtual void visitPostfix(struct PostfixExpr &expr) = 0;
		virtual void visitShift(struct ShiftExpr &expr) = 0;
		virtual void visitThis(struct ThisExpr &expr) = 0;
	};
	struct ValueExpr : public Expr {
		Token value;
		explicit ValueExpr(Token t) : value(std::move(t)) {}

		void accept(ExprVisitor &visitor) override { visitor.visitValue(*this); }
	};
	struct MathExpr : public Expr {
		std::shared_ptr<Expr> left;
		Token op_t;
		std::shared_ptr<Expr> right;

		MathExpr(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r) :
				left(std::move(l)), op_t(std::move(op)), right(std::move(r)) {}

		void accept(ExprVisitor &visitor) override { visitor.visitMath(*this); }
	};
	struct GroupExpr : public Expr {
		std::shared_ptr<Expr> expression;

		explicit GroupExpr(std::shared_ptr<Expr> e) : expression(std::move(e)) {}

		void accept(ExprVisitor &visitor) override { visitor.visitGroup(*this); }
	};
	struct PrefixExpr : public Expr {
		Token prefix;
		std::shared_ptr<Expr> right;

		explicit PrefixExpr(Token p, std::shared_ptr<Expr> r) : prefix(std::move(p)), right(std::move(r)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitPrefix(*this); }
	};
	struct PostfixExpr : public Expr {
		Token postfix;
		std::shared_ptr<Expr> left;
		explicit PostfixExpr(Token p, std::shared_ptr<Expr> l) : postfix(std::move(p)), left(std::move(l)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitPostfix(*this); }
	};
	struct VariableExpr : public Expr {
		Token name;
		explicit VariableExpr(Token n) : name(std::move(n)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitVariable(*this); }
	};
	struct AssignExpr : public Expr {
		Token name;
		std::shared_ptr<Expr> value;
		AssignExpr(Token n, std::shared_ptr<Expr> v) : name(std::move(n)), value(std::move(v)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitAssign(*this); }
	};
	struct LogicalExpr : public Expr {
		std::shared_ptr<Expr> left;
		Token op_t;
		std::shared_ptr<Expr> right;
		LogicalExpr(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r) :
				left(std::move(l)), op_t(std::move(op)), right(std::move(r)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitLogical(*this); }
	};
	struct CallExpr : public Expr {
		std::shared_ptr<Expr> callee;
		std::vector<std::shared_ptr<Expr>> arguments;
		Token Paren;
		CallExpr(std::shared_ptr<Expr> c, std::vector<std::shared_ptr<Expr>> args, Token p) :
				callee(std::move(c)), arguments(std::move(args)), Paren(std::move(p)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitCall(*this); }
	};
	struct ListExpr : public Expr {
		std::vector<std::shared_ptr<Expr>> elements;
		ListExpr(std::vector<std::shared_ptr<Expr>> elements) : elements(std::move(elements)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitList(*this); }
	};

	struct IndexExpr : public Expr {
		std::shared_ptr<Expr> object;
		std::shared_ptr<Expr> index;
		Token bracket;
		IndexExpr(std::shared_ptr<Expr> o, std::shared_ptr<Expr> i, Token b) :
				object(std::move(o)), index(std::move(i)), bracket(std::move(b)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitIndex(*this); }
	};

	struct GetExpr : public Expr {
		std::shared_ptr<Expr> object;
		Token name;
		GetExpr(std::shared_ptr<Expr> o, Token n) : object(std::move(o)), name(std::move(n)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitGet(*this); }
	};

	struct SetExpr : public Expr {
		std::shared_ptr<Expr> object;
		Token name;
		std::shared_ptr<Expr> value;
		SetExpr(std::shared_ptr<Expr> o, Token n, std::shared_ptr<Expr> v) :
				object(std::move(o)), name(std::move(n)), value(std::move(v)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitSet(*this); }
	};
	struct MapExpr : public Expr {
		Token braceToken;
		// A vector of pairs: first is the Key expression, second is the Value expression
		std::vector<std::pair<std::shared_ptr<Expr>, std::shared_ptr<Expr>>> items;

		MapExpr(Token braceTok, std::vector<std::pair<std::shared_ptr<Expr>, std::shared_ptr<Expr>>> items) :
				braceToken(std::move(braceTok)), items(std::move(items)) {}

		void accept(ExprVisitor &visitor) override { visitor.visitMap(*this); }
	};
	struct IndexSetExpr : public Expr {
		std::shared_ptr<Expr> object;
		Token bracket;
		std::shared_ptr<Expr> index;
		std::shared_ptr<Expr> value;

		IndexSetExpr(std::shared_ptr<Expr> object, Token bracket, std::shared_ptr<Expr> index,
								 std::shared_ptr<Expr> value) : object(object), bracket(bracket), index(index), value(value) {}

		void accept(ExprVisitor &visitor) override { visitor.visitIndexSet(*this); }
	};
	struct RangeExpr : public Expr {
		std::shared_ptr<Expr> leftBound;
		Token op_t;
		std::shared_ptr<Expr> rightBound;
		RangeExpr(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r) :
				leftBound(std::move(l)), op_t(std::move(op)), rightBound(std::move(r)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitRange(*this); }
	};
	struct BitwiseAndExpr : public Expr {
		std::shared_ptr<Expr> left;
		Token op_t;
		std::shared_ptr<Expr> right;
		BitwiseAndExpr(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r) :
				left(std::move(l)), op_t(std::move(op)), right(std::move(r)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitBitwiseAnd(*this); }
	};
	struct BitwiseOrExpr : public Expr {
		std::shared_ptr<Expr> left;
		Token op_t;
		std::shared_ptr<Expr> right;
		BitwiseOrExpr(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r) :
				left(std::move(l)), op_t(std::move(op)), right(std::move(r)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitBitwiseOr(*this); }
	};
	struct BitwiseXorExpr : public Expr {
		std::shared_ptr<Expr> left;
		Token op_t;
		std::shared_ptr<Expr> right;
		BitwiseXorExpr(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r) :
				left(std::move(l)), op_t(std::move(op)), right(std::move(r)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitBitwiseXor(*this); }
	};
	struct ShiftExpr : public Expr {
		std::shared_ptr<Expr> left;
		Token op_t;
		std::shared_ptr<Expr> right;
		ShiftExpr(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r) :
				left(std::move(l)), op_t(std::move(op)), right(std::move(r)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitShift(*this); }
	};
	struct ThisExpr  : public Expr {
		Token keyword;
		explicit ThisExpr(Token k) : keyword(std::move(k)) {}
		void accept(ExprVisitor &visitor) override { visitor.visitThis(*this); }
	};
} // namespace Backend
