#ifndef _UNARY_EXPR_H_
#define _UNARY_EXPR_H_

#include "Tag.h"
#include "Expr.h"
#include "visitors/Visitor.h"
#include <memory>


class UnaryExpr : public Expr
{
private:
    friend class Evaluator;
    Tag op;
    std::shared_ptr<Expr> content{};

public:
    UnaryExpr(Tag t, std::shared_ptr<Expr>&& c) :
        op(t), content(c), Expr(Which::unary) {}

    bool TypeChecking();
    void Accept(Visitor* v) { v->VisitUnaryExpr(this); }
};

#endif // _UNARY_EXPR_H_
