#ifndef _INITDECL_H_
#define _INITDECL_H_

#include <memory>
#include "../Node.h"
#include "Declarator.h"
#include "Init.h"

class InitDecl : public Node
{
public:
    std::unique_ptr<Declarator> declarator{ nullptr };
    std::unique_ptr<Init> initializer{ nullptr };

    InitDecl(std::unique_ptr<Declarator>&);
    InitDecl(std::unique_ptr<Declarator>&, std::unique_ptr<Init>&);

    const std::string& GetName();
};

#endif // _INITDECL_H_
