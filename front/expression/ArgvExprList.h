#ifndef _ARGVEXPRLIST_H_
#define _ARGVEXPRLIST_H_

#include "../Node.h"
#include <list>

class AssignExpr;
#include "AssignExpr.h"

class ArgvExprList : public Node
{
public:
    std::list<AssignExpr> argvExprList{};
};

#endif // _ARGVEXPRLIST_H_
