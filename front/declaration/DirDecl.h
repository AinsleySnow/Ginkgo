#ifndef _DIRDECL_H_
#define _DIRDECL_H_

#include <string>
#include "Node.h"

class DirDecl : Node
{
private:
    std::string identifier{};

public:
    DirDecl(const std::string&);
    DirDecl(const Node&);
    DirDecl(const DirDecl&) = default;

    std::string GetIdentifier() const;
};

#endif // _DIRDECL_H_
