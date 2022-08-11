#ifndef _IR_H_
#define _IR_H_

#include <list>
#include <string>
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
};

#endif // _IR_H_
