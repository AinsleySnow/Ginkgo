#ifndef _IR_H_
#define _IR_H_

#include <vector>
#include <string>
#include "op.h"

struct Triple
{
    Op op;
    std::string arg1;
    std::string arg2;

    std::string ToString();
};

using IRIter = std::vector<Triple>::const_iterator;

class IR
{
private:
    std::vector<Triple> data {};

public:
    void Append(const Triple&);
    IRIter iter();
};

#endif // _IR_H_
