#pragma once
#include "expr.h"
#include <memory>

namespace Backend {
    // Inherit publicly from ExprVisitor
    class Optimizer : public ExprVisitor {
    public:
        std::shared_ptr<Expr> fold(std::shared_ptr<Expr> expr) {
            expr->accept(*this);
            return lastFolded;
        }

        		void visitValue(ValueExpr &expr) override;
		void visitMath(MathExpr &expr) override;
		void visitBitwiseOr(BitwiseOrExpr &expr) override;
		void visitBitwiseXor(BitwiseXorExpr &expr) override;
		void visitBitwiseAnd(BitwiseAndExpr &expr) override;
		void visitPrefix(PrefixExpr &expr) override;
		void visitPostfix(PostfixExpr &expr) override;
		void visitShift(ShiftExpr &expr) override;
		void visitGroup(GroupExpr &expr) override;
		void visitVariable(VariableExpr &expr) override;
		void visitLogical(LogicalExpr &expr) override;
		void visitAssign(AssignExpr &expr) override;
		void visitCall(CallExpr &expr) override;
		void visitThis(ThisExpr &expr) override;
		void visitGet(GetExpr &expr) override;
		void visitMap(MapExpr &expr) override;
		void visitRange(RangeExpr &expr) override;
		void visitSet(SetExpr &expr) override;
    void visitIndexSet(IndexSetExpr &expr) override;
    void visitIndex(IndexExpr &expr) override;
    void visitList(ListExpr &expr) override;
    

    private:
        std::shared_ptr<Expr> lastFolded;
    };
}