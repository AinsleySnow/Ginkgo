#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Expr.h"
#include "Identifier.h"
#include "visitors/Visitor.h"
#include <memory>
#include <string>


class Object : public Identifier
{
private:
    std::shared_ptr<Expr> val_{};

public:
    Object(Identifier* ident) : Identifier(*ident) 
    { identype_ = IdentType::obj; }

    void SetValExpr(std::shared_ptr<Expr> expr) { val_ = expr; }

    void Accept(Visitor* v) { v->VisitObject(this); }
};

#endif // _OBJECT_H_
