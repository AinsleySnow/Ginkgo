#ifndef _DECLARATION_H_
#define _DECLARATION_H_

#include <memory>
#include "../Node.h"
#include "DeclSpec.h"
#include "InitDeclList.h"

class Declaration : public Node
{
public:
    std::unique_ptr<InitDeclList> initDeclList{ nullptr };
    std::unique_ptr<DeclSpec> declSpec{ nullptr };
    
    Declaration(std::unique_ptr<DeclSpec>&&, std::unique_ptr<InitDeclList>&&);
};

#endif // _DECLARATION_H_
