#ifndef _DECLARATION_H_
#define _DECLARATION_H_

#include "Node.h"
#include "InitDeclList.h"
#include "DeclSpec.h"

class Declaration : Node
{
private:
    InitDeclList initDeclList;
    DeclSpec declSpec;

public:
    Declaration(const InitDeclList&, const DeclSpec&);

    DeclSpec GetSpecifier() const;
    InitDeclList GetInitList() const;
};

#endif // _DECLARATION_H_
