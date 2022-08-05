#ifndef _INITDECL_H_
#define _INITDECL_H_

#include "../Node.h"
#include "Declarator.h"
#include "Init.h"

class InitDecl : public Node
{
private:
    Declarator declarator;
    Init initializer;

public:
    InitDecl(const Declarator&);
    InitDecl(const Declarator&, const Init&);

    std::string GetName() const;
    Init GetInitializer() const;
};

#endif // _INITDECL_H_
