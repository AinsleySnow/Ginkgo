#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

#include "Expr.h"
#include "types/Type.h"
#include <memory>
#include <string>

class Object;
class Func;
class Label;
class Typedef;
class Member;


class Identifier : public Expr
{
public:
    Identifier() {}
    Identifier(const std::string& n) : name_(n) {}
    virtual ~Identifier() {};

    std::string GetName() const { return name_; }

    Identifier* ToIdentifier() override { return this; }

    virtual Object* ToObject() { return nullptr; }
    virtual Func* ToFunc() { return nullptr; }
    virtual Typedef* ToTypedef() { return nullptr; }
    virtual Member* ToMember() { return nullptr; }
    virtual Label* ToLabel() { return nullptr; }

protected:
    std::string name_{};
};


class Func : public Identifier
{
public:
    Func(const std::string& n) : Identifier(n) {}
    Func(const std::string& n, std::shared_ptr<FuncType> t) :
        Identifier(n), functype_(t) {}

    Func* ToFunc() override { return this; }


private:
    std::shared_ptr<FuncType> functype_{};
};


class Label : public Identifier
{
public:
    Label(const std::string& n) : Identifier(n) {}

    void Accept(Visitor* v) override { v->VisitLabel(this); }
    Label* ToLabel() override { return this; }
};


class Object : public Identifier
{
private:
    std::shared_ptr<Expr> val_{};

public:
    Object(const std::string& n) : Identifier(n) {}
    Object(std::shared_ptr<Identifier> ident) : Identifier(*ident) {}

    Object* ToObject() override { return this; }
    Constant* ToConstant() override
    {
        if (GetType()->GetQual().IsConstant())
            return val_->ToConstant();
        return nullptr;
    }

    bool IsLVal() override { return true; }
    void Accept(Visitor* v) override { v->VisitObject(this); }

    void SetInit(std::shared_ptr<Expr> expr) { val_ = expr; }
    const Expr* GetInit() const { return val_.get(); }
};




#endif // _IDENTIFIER_H_
