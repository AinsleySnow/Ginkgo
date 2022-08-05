#ifndef _DIRDECL_H_
#define _DIRDECL_H_

#include <string>
#include "../Node.h"

class DirDecl : public Node
{
private:
    std::string identifier{};

public:
    DirDecl();
    DirDecl(const std::string&);
    DirDecl(const Node&);
    DirDecl(const DirDecl&) = default;

    std::string GetIdentifier() const;
};

#endif // _DIRDECL_H_
