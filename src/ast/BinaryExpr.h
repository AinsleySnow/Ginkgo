#ifndef _BINARY_EXPR_H_
#define _BINARY_EXPR_H_

#include "Expr.h"
#include "Tag.h"
#include "visitors/Visitor.h"
#include <memory>


class BinaryExpr : public Expr
{
private:
    friend class IRGen;
    std::shared_ptr<Expr> left_{};
    Tag op_;
    std::shared_ptr<Expr> right_{};


public:
    BinaryExpr(std::shared_ptr<Expr>&& l, Tag t, std::shared_ptr<Expr>&& r) :
        left_(l), op_(t), right_(r) {}

    void Accept(Visitor* v) override { v->VisitBinaryExpr(this); }
    BinaryExpr* ToBinary() override { return this; }
};

#endif // _BINARY_EXPR_H_
