#ifndef _IR_H_
#define _IR_H_

#include <vector>
#include <string>
#include "op.h"

struct triple
{
    op op;
    std::string arg1;
    std::string arg2;

    std::string ToString();
};

using IRIter = std::vector<triple>::const_iterator;

class IR
{
private:
    std::vector<triple> data {};

public:
    void Append(const triple&);
    IRIter iter();
};

#endif // _IR_H_
