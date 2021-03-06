#include "InitDeclList.h"

InitDeclList::InitDeclList() : Node(Tag::init_declarator_list) {}

void InitDeclList::Append(const InitDecl& id)
{
    initList.push_back(id);
}

void InitDeclList::Join(InitDeclList& idl)
{
    initList.merge(idl.initList);
}

std::list<InitDecl> InitDeclList::GetIter()
{
    return initList;
}
