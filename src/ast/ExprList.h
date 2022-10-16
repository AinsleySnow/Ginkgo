#ifndef _EXPR_LIST_H_
#define _EXPR_LIST_H_

#include "Expr.h"
#include "visitors/Visitor.h"
#include <deque>
#include <memory>


class ExprList : public Expr
{
private:
    std::deque<std::shared_ptr<Expr>> exprlist_{};

public:
    ExprList() : Expr(Which::exprlist) {}

    void Append(Expr* expr) { exprlist_.push_back(std::shared_ptr<Expr>(expr)); }
    auto begin() { return exprlist_.begin(); }
    auto end() { return exprlist_.end(); }
};

#endif // _EXPR_LIST_H_
