#ifndef _ASSIGN_EXPR_H_
#define _ASSIGN_EXPR_H_

#include "Expr.h"

class UnaryExpr;
class CondExpr;
#include "CondExpr.h"
#include "UnaryExpr.h"

class AssignExpr : public Expr
{
private:
    std::shared_ptr<Expr> left_{};
    Tag op_;
    std::shared_ptr<Expr> right_{};

public:
    AssignExpr(std::shared_ptr<Expr>&& l, Tag t, std::shared_ptr<Expr>&& r) :
        left_(l), op_(t), right_(r), Expr(Which::assign) {}

    bool TypeChecking();
    void Accept(Visitor* v) { v->VisitAssignExpr(this); }
};

#endif // _ASSIGN_EXPR_H_
