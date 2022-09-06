#ifndef _INIT_H_
#define _INIT_H_

#include "../Node.h"
#include "../expression/AssignExpr.h"

class Init : public Node
{
public:
    std::unique_ptr<AssignExpr> assignExpr;
    std::unique_ptr<InitList> initList;

    Init(std::unique_ptr<InitList>&& il) :
        initList(std::move(il)) {};
    Init(std::unique_ptr<AssignExpr>&& ae) :
        assignExpr(std::move(ae)) {};
};

#endif // _INIT_H_
