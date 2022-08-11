#ifndef _IR_H_
#define _IR_H_

#include <list>
#include <string>
#include <string_view>
#include "op.h"

struct Quadruple
{
    Op op;
    std::string arg1;
    std::string arg2;
    std::string arg3;

    std::string ToString();
};

class IR
{
private:
    std::list<Quadruple> data {};

public:
    void Append(const Quadruple&);
    void Join(IR&);
    std::string_view GetLastVar() const;
};

#endif // _IR_H_
