#ifndef _EXPR_H_
#define _EXPR_H_

#include "ast/CType.h"
#include <memory>

class IROperand;
class Visitor;


class Expr
{
public:
    Expr() {}
    Expr(Expr&&) = default;
    Expr& operator=(Expr&&) = default;
    virtual ~Expr() {}

    virtual void Accept(Visitor*) {}
    virtual bool IsLVal() const { return false; }

    const auto& Type() const { return type_; }
    auto& Type() { return type_; }
    const CType* RawType() const { return type_.get(); }
    auto Val() const { return val_; }
    auto& Val() { return val_; }


protected:
    const IROperand* val_{};
    std::unique_ptr<CType> type_;
};

#endif // _EXPR_H_
