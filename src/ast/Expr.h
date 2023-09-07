#ifndef _EXPR_H_
#define _EXPR_H_

#include "ast/CType.h"
#include <memory>

class AccessExpr;
class ArrayExpr;
class ConstExpr;
class ExprList;
class IdentExpr;
class StrExpr;
class UnaryExpr;
class IROperand;
class ASTVisitor;


class Expr
{
public:
    virtual ~Expr() {}

    virtual void Accept(ASTVisitor*) {}
    virtual bool IsLVal() const { return false; }
    virtual bool IsConstant() const { return false; }
    virtual bool IsAccess() const { return false; }
    virtual bool IsExprList() const { return false; }
    virtual bool IsIdentifier() const { return false; }
    virtual bool IsStrExpr() const { return false; }
    virtual bool IsSubscript() const { return false; }
    virtual bool IsUnary() const { return false; }

    virtual ConstExpr* ToConstant() { return nullptr; }
    virtual AccessExpr* ToAccess() { return nullptr; }
    virtual ExprList* ToExprList() { return nullptr; }
    virtual IdentExpr* ToIdentifier() { return nullptr; }
    virtual ArrayExpr* ToSubscript() { return nullptr; }
    virtual StrExpr* ToStrExpr() { return nullptr; }
    virtual UnaryExpr* ToUnary() { return nullptr; }
    virtual const ConstExpr* ToConstant() const { return nullptr; }
    virtual const AccessExpr* ToAccess() const { return nullptr; }
    virtual const ExprList* ToExprList() const { return nullptr; }
    virtual const IdentExpr* ToIdentifier() const { return nullptr; }
    virtual const ArrayExpr* ToSubscript() const { return nullptr; }
    virtual const StrExpr* ToStrExpr() const { return nullptr; }
    virtual const UnaryExpr* ToUnary() const { return nullptr; }

    const auto& Type() const { return type_; }
    auto& Type() { return type_; }
    const CType* RawType() const { return type_.get(); }
    auto Val() const { return val_; }
    auto& Val() { return val_; }


protected:
    const IROperand* val_{};
    std::shared_ptr<CType> type_;
};

#endif // _EXPR_H_
