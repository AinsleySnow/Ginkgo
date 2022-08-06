#ifndef _DECLARATOR_H_
#define _DECLARATOR_H_

#include <memory>
#include "../Node.h"
#include "DirDecl.h"
#include "Ptr.h"

class Declarator : public Node
{
public:
    Declarator();
    Declarator(std::unique_ptr<DirDecl>&);
    Declarator(std::unique_ptr<Ptr>&, std::unique_ptr<DirDecl>&);

    std::unique_ptr<DirDecl> directdecl{ nullptr };
    std::unique_ptr<Ptr> ptr{ nullptr };
};

#endif // _DECLARATOR_H_
