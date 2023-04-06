#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

#include "ast/CType.h"
#include "ast/Expr.h"
#include <memory>
#include <string>

class Constant;
class Object;
class Func;
class Label;
class Typedef;
class CustomedType;
class Member;
class Register;


class Identifier
{
public:
    enum class IdentType { obj, func, label, tydef, custom, member };

    Identifier(IdentType it, const std::string& n) : identype_(it), name_(n) {}
    Identifier(IdentType it, const std::string& n, const CType* t) :
        identype_(it), name_(n), type_(t) {}
    virtual ~Identifier() {}

    std::string GetName() const { return name_; }
    auto GetIdentType() const { return identype_; }
    const CType* GetCType() const { return type_; }

    virtual Object* ToObject() { return nullptr; }
    virtual Func* ToFunc() { return nullptr; }
    virtual Typedef* ToTypedef() { return nullptr; }
    virtual CustomedType* ToCustomed() { return nullptr; }
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
    Func(const std::string& n, const CFuncType* t, const Register* addr) :
        Identifier(Identifier::IdentType::func, n, t), addr_(addr) {}

    Func* ToFunc() override { return this; }
    auto Addr() const { return addr_; }

private:
    const Register* addr_{};
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

    bool IsConst() const { return value_; }
    auto Addr() const { return reg_; }
    auto& Value() { return value_; }
    auto Value() const { return value_; }

private:
    const Register* reg_{};
    const Constant* value_{};
};


class CustomedType : public Identifier
{
public:
    CustomedType(const std::string& n, const CType* ty) :
        Identifier(Identifier::IdentType::custom, n, ty) {}

    CustomedType* ToCustomed() override { return this; }
};


class Member : public Identifier
{
public:
    Member(const std::string& n, const CType* ty) :
        Identifier(Identifier::IdentType::member, n, ty) {}

    Member* ToMember() override { return this; }
    auto Value() const { return value_; }
    auto& Value() { return value_; }

private:
    // This field can only be set
    // if the instance has enum type.
    const IntConst* value_{};
};


#endif // _IDENTIFIER_H_
