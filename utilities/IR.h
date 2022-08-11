#ifndef _IR_H_
#define _IR_H_

#include <list>
#include <string>
#include "op.h"

struct Triple
{
    Op op;
    std::string arg1;
    std::string arg2;

    std::string ToString();
};

class IR
{
private:
    std::list<Triple> data {};

public:
    void Append(const Triple&);
    void Join(IR&);
};

#endif // _IR_H_
