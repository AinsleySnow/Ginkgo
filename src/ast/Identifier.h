#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

#include "Expr.h"
#include "types/Type.h"
#include <memory>

class Object;
class Func;
class Typedef;
class Member;


class Identifier : public Expr
{
public:
    enum class IdentType { obj, func, tag, mem, _typedef, undef };

    Identifier(std::string* n) :
        name_(*n),
        Expr(Which::identifier),
        identype_(IdentType::undef) {};
    virtual ~Identifier() {};

    std::string GetName() const { return name_; }
    IdentType GetIdentType() const { return identype_; }

    virtual Object* ToObject() { return nullptr; }
    virtual Func* ToFunc() { return nullptr; }
    virtual Typedef* ToTypedef() { return nullptr; }
    virtual Member* ToMember() { return nullptr; }

protected:
    std::string name_{};
    IdentType identype_{};
};

#endif // _IDENTIFIER_H_
