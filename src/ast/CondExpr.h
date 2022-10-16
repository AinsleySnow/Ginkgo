#ifndef _COND_EXPR_H_
#define _COND_EXPR_H_

#include "Expr.h"
#include <memory>


class CondExpr : public Expr
{
private:
    std::shared_ptr<Expr> cond_{};
    std::shared_ptr<Expr> true_{};
    std::shared_ptr<Expr> false_{};

public:
    CondExpr(std::shared_ptr<Expr>&& c, std::shared_ptr<Expr>&& t,
        std::shared_ptr<Expr>&& f) :
        cond_(c), true_(t), false_(f), Expr(Which::condition) {}
};

#endif // _COND_EXPR_H_
