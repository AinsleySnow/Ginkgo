#ifndef _STR_H_
#define _STR_H_

#include "Expr.h"
#include "visitors/Visitor.h"
#include <string>
#include <memory>


class Str : public Expr
{
private:
    std::string content_{};

public:
    Str(std::string s) : content_(s) {}

    void Accept(Visitor* v) override { v->VisitStr(this); }
};


#endif // _STR_H_
