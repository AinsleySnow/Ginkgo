#include "InitDecl.h"

InitDecl::InitDecl(const Declarator& d) : Node(Tag::init_declarator)
{
    
}

InitDecl::InitDecl(const Declarator& d, const Init& i) : Node(Tag::init_declarator)
{

}
