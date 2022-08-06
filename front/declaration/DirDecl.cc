#include "DirDecl.h"

DirDecl::DirDecl(const std::string& s)
{
    identifier = s;
}

DirDecl::DirDecl(std::unique_ptr<DirDecl>& dd)
{
    dirDecl = std::move(dd);
}
