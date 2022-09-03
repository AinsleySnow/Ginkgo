#ifndef _ARGVEXPRLIST_H_
#define _ARGVEXPRLIST_H_

#include "../Node.h"
#include "AssignExpr.h"
#include <list>

class ArgvExprList : public Node
{
public:
    std::list<AssignExpr> argvExprList{};
};

#endif // _ARGVEXPRLIST_H_
