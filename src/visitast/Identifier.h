#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

#include "ast/CType.h"
#include "ast/Expr.h"
#include <memory>
#include <string>

class Object;
class Func;
class Label;
class Typedef;
class Member;
class Register;


class Identifier
{
public:
    enum class IdentType { obj, func, label, tydef, member };

    Identifier(IdentType it, const std::string& n) : name_(n) {}
    Identifier(IdentType it, const std::string& n, const CType* t) :
        name_(n), type_(t) {}
    virtual ~Identifier() {}

    std::string GetName() const { return name_; }
    auto GetIdentType() const { return identype_; }
    const CType* GetCType() const { return type_; }

    virtual Object* ToObject() { return nullptr; }
    virtual Func* ToFunc() { return nullptr; }
    virtual Typedef* ToTypedef() { return nullptr; }
    virtual Member* ToMember() { return nullptr; }
    virtual Label* ToLabel() { return nullptr; }


protected:
    std::string name_{};
    const CType* type_{};


private:
    IdentType identype_{};
};


class Func : public Identifier
{
public:
    Func(const std::string& n, const CFuncType* t) :
        Identifier(Identifier::IdentType::func, n, t) {}

    Func* ToFunc() override { return this; }
};


class Label : public Identifier
{
public:
    Label(const std::string& n) :
        Identifier(Identifier::IdentType::label, n) {}

    Label* ToLabel() override { return this; }
};


class Object : public Identifier
{
public:
    Object(const std::string& n, const CType* t, const Register* r) :
        Identifier(Identifier::IdentType::obj, n, t), reg_(r) {}

    Object* ToObject() override { return this; }
    auto GetAddr() const { return reg_; }

private:
    const Register* reg_{};
};


#endif // _IDENTIFIER_H_
