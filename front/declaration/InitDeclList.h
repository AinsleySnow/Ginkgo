#ifndef _INITDECLLIST_H_
#define _INITDECLLIST_H_

#include <list>
#include "../Node.h"
#include "InitDecl.h"

using ListIter = std::list<InitDecl>::iterator;

class InitDeclList : public Node
{
private:
    std::list<InitDecl> initList{};
    
public:
    InitDeclList();

    void Append(const InitDecl&);
    void Join(InitDeclList&);
    std::list<InitDecl> GetIter();
};

#endif // _INITDECLLIST_H_
