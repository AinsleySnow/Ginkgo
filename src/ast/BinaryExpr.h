#ifndef _BINARY_EXPR_H_
#define _BINARY_EXPR_H_

#include "Expr.h"
#include "Tag.h"
#include "visitors/Visitor.h"
#include <memory>


class BinaryExpr : public Expr
{
private:
    friend class Evaluator;
    std::shared_ptr<Expr> left{};
    Tag op;
    std::shared_ptr<Expr> right{};

public:
    BinaryExpr(std::shared_ptr<Expr>&& l, Tag t, std::shared_ptr<Expr>&& r) :
        left(l), op(t), right(r), Expr(Which::binary) {}

    bool TypeChecking() override;
    void Accept(Visitor* v) { v->VisitBinaryExpr(this); };
};

#endif // _BINARY_EXPR_H_
