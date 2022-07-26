#ifndef _DECLARATOR_H_
#define _DECLARATOR_H_

#include "Node.h"
#include "DirDecl.h"

class Declarator : Node
{
private:
    bool withPtr { false };
    DirDecl directdecl;
    
public:
    Declarator(const DirDecl&);
};

#endif // _DECLARATOR_H_
