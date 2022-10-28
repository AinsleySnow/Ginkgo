#ifndef _UNARY_EXPR_H_
#define _UNARY_EXPR_H_

#include "Tag.h"
#include "Expr.h"
#include "visitors/Visitor.h"
#include <memory>


class UnaryExpr : public Expr
{
private:
    friend class IRGen;
    Tag op_;
    std::shared_ptr<Expr> content_{};


public:
    UnaryExpr(Tag t, std::shared_ptr<Expr> c) :
        op_(t), content_(c) {}

    void Accept(Visitor* v) override { v->VisitUnaryExpr(this); }
    UnaryExpr* ToUnary() override { return this; }
};

#endif // _UNARY_EXPR_H_
