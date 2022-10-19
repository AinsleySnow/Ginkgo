#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

#include "Expr.h"
#include "types/Type.h"
#include <memory>

class Object;
class Func;
class Label;
class Typedef;
class Member;


class Identifier : public Expr
{
public:
    Identifier(const std::string& n) : name_(n) {};
    virtual ~Identifier() {};

    std::string GetName() const { return name_; }

    virtual Object* ToObject() { return nullptr; }
    virtual Func* ToFunc() { return nullptr; }
    virtual Typedef* ToTypedef() { return nullptr; }
    virtual Member* ToMember() { return nullptr; }
    virtual Label* ToLabel() { return nullptr; }

protected:
    std::string name_{};
};

#endif // _IDENTIFIER_H_
