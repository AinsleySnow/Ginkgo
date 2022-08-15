#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "AssignExpr.h"
#include <list>

class Expression : public Node, public IGenerable
{
public:
    std::list<AssignExpr> exprs{};

    IR Generate(SymbolTable&) const override;
};

#endif // _EXPRESSION_H_
