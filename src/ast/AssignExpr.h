#ifndef _ASSIGN_EXPR_H_
#define _ASSIGN_EXPR_H_

#include "Expr.h"
#include "visitors/Visitor.h"


class AssignExpr : public Expr
{
private:
    friend class IRGen;
    std::shared_ptr<Expr> left_{};
    Tag op_;
    std::shared_ptr<Expr> right_{};

public:
    AssignExpr(std::shared_ptr<Expr> l, Tag t, std::shared_ptr<Expr> r) :
        left_(l), op_(t), right_(r) {}

    void Accept(Visitor* v) override { v->VisitAssignExpr(this); }
    AssignExpr* ToAssign() override { return this; }
};

#endif // _ASSIGN_EXPR_H_
