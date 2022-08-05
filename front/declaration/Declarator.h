#ifndef _DECLARATOR_H_
#define _DECLARATOR_H_

#include "../Node.h"
#include "DirDecl.h"

class Declarator : public Node
{
private:
    bool withPtr { false };
    DirDecl directdecl;
    
public:
    Declarator();
    Declarator(const DirDecl&);

    std::string GetName() const;
};

#endif // _DECLARATOR_H_
