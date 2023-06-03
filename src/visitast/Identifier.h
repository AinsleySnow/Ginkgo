#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

#include "ast/CType.h"
#include "ast/Expr.h"
#include "utils/DynCast.h"
#include <memory>
#include <string>
#include <variant>

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

    virtual std::unique_ptr<Identifier>&& Clone() { return nullptr; }

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

    std::unique_ptr<Identifier>&& Clone() { return std::make_unique<Func>(*this); }

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

    std::unique_ptr<Identifier>&& Clone() { return std::make_unique<Label>(*this); }

    Label* ToLabel() override { return this; }
};


class Object : public Identifier
{
public:
    Object(const std::string& n, const CType* t, const Register* r) :
        Identifier(Identifier::IdentType::obj, n, t), reg_(r) {}

    std::unique_ptr<Identifier>&& Clone() { return std::make_unique<Object>(*this); }

    Object* ToObject() override { return this; }

    bool IsConst() const { return value_; }
    auto Addr() const { return reg_; }
    auto& Value() { return value_; }
    auto Value() const { return value_; }

private:
    const Register* reg_{};
    const Constant* value_{};
};


class Typedef : public Identifier
{
public:
    Typedef(const std::string& n, const CType* ty) :
        Identifier(Identifier::IdentType::tydef, n, ty), type_(ty) {}
    Typedef(const std::string& n, const Typedef* t) :
        Identifier(Identifier::IdentType::tydef, n, nullptr), type_(t) {}

    std::unique_ptr<Identifier>&& Clone() { return std::make_unique<Typedef>(*this); }

    Typedef* ToTypedef() override { return this; }
    auto& Type() { return type_; }
    auto Type() const { return type_; }

private:
    std::variant<const CType*, const Typedef*> type_{};
};


class CustomedType : public Identifier
{
public:
    CustomedType(const std::string& n, const CType* ty) :
        Identifier(Identifier::IdentType::custom, n, ty) {}

    std::unique_ptr<Identifier>&& Clone() { return std::make_unique<CustomedType>(*this); }

    CustomedType* ToCustomed() override { return this; }
};


class Member : public Identifier
{
public:
    Member(const std::string& n, const CType* ty) :
        Identifier(Identifier::IdentType::member, n, ty) {}

    std::unique_ptr<Identifier>&& Clone() { return std::make_unique<Member>(*this); }

    Member* ToMember() override { return this; }
    auto Value() const { return value_; }
    auto& Value() { return value_; }

private:
    // This field can only be set
    // if the instance has enum type.
    const IntConst* value_{};
};


#endif // _IDENTIFIER_H_
