#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Expr.h"
#include "Identifier.h"
#include "types/ArithmType.h"
#include "visitors/Visitor.h"
#include <memory>
#include <string>


class Object : public Identifier
{
private:
    std::shared_ptr<Expr> val_{};

public:
    Object(const std::string& n) : Identifier(n) {}
    Object(std::shared_ptr<Identifier> ident) : Identifier(*ident) {}

    Object* ToObject() override { return this; }
    // I don't think implement this method here is a good practice.
    // But it does simplify many operations.
    Constant* ToConstant() override { return val_->ToConstant(); }
    bool IsLVal() override { return true; }

    void SetValExpr(std::shared_ptr<Expr> expr) { val_ = expr; }

    void Accept(Visitor* v) override { v->VisitObject(this); }
};

#endif // _OBJECT_H_
