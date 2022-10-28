#ifndef _EXPR_H_
#define _EXPR_H_

#include "types/Type.h"
#include <memory>

class AssignExpr;
class BinaryExpr;
class CondExpr;
class Constant;
class ExprList;
class Identifier;
class Str;
class UnaryExpr;
class Visitor;


class Expr
{
public:   
    virtual ~Expr() {}

    virtual bool TypeChecking() { return false; }
    virtual bool IsLVal() { return false; }
    
    virtual void Accept(Visitor*) {}

    virtual AssignExpr* ToAssign() { return nullptr; }
    virtual BinaryExpr* ToBinary() { return nullptr; }
    virtual Constant* ToConstant() { return nullptr; }
    virtual CondExpr* ToCondition() { return nullptr; }
    virtual ExprList* ToExprList() { return nullptr; }
    virtual Identifier* ToIdentifier() { return nullptr; }
    virtual Str* ToStr() { return nullptr; }
    virtual UnaryExpr* ToUnary() { return nullptr; }

    void SetType(std::shared_ptr<Type> t) { type_ = t; }
    std::shared_ptr<Type> GetType() const { return type_; }

protected:
    std::shared_ptr<Type> type_;
};

#endif // _EXPR_H_
