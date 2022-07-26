#ifndef _INITDECL_H_
#define _INITDECL_H_

#include "Node.h"
#include "Declarator.h"
#include "Init.h"

class InitDecl : Node
{
private:
    Declarator declarator;
    Init initializer;

public:
    InitDecl(const Declarator&);
    InitDecl(const Declarator&, const Init&);
};

#endif // _INITDECL_H_
