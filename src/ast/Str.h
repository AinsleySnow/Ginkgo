#ifndef _STR_H_
#define _STR_H_

#include "Expr.h"
#include <string>
#include <memory>


class Str : public Expr
{
private:
    std::shared_ptr<std::string> content{};

public:
    Str(std::string* s) :
        content(std::shared_ptr<std::string>(s)),
        Expr(Which::string) {}
};


#endif // _STR_H_
