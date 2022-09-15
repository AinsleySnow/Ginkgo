#ifndef _INITDECLLIST_H_
#define _INITDECLLIST_H_

#include <list>
#include "../Node.h"

class InitDecl;
#include "InitDecl.h"

class InitDeclList : public Node
{
public:
    std::list<InitDecl> initList{};
};

#endif // _INITDECLLIST_H_
