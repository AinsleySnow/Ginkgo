#ifndef _DIRDECL_H_
#define _DIRDECL_H_

#include <memory>
#include <string>
#include "../Node.h"

class DirDecl : public Node
{
public:
    std::string identifier{};
    std::unique_ptr<DirDecl> dirDecl{ nullptr };

    DirDecl();
    DirDecl(const std::string&);
    DirDecl(std::unique_ptr<DirDecl>&);
};

#endif // _DIRDECL_H_
