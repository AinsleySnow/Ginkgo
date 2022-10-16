#ifndef _FUNCDEF_H_
#define _FUNCDEF_H_

#include <memory>

class DeclSpec;
#include "DeclSpec.h"
class Declarator;
#include "Declarator.h"
class CmpdStmt;
#include "../statement/Scope.h"


class FuncDef
{
public:
    std::unique_ptr<DeclSpec> declSpec{};
    std::unique_ptr<Declarator> declarator{};
    std::unique_ptr<CmpdStmt> cmpdStmt{};

    FuncDef(
        std::unique_ptr<DeclSpec>&& ds,
        std::unique_ptr<Declarator>&& d,
        std::unique_ptr<CmpdStmt>&& cs
    ) : declSpec(std::move(ds)),
        declarator(std::move(d)),
        cmpdStmt(std::move(cs)) {};
};

#endif // _FUNCDEF_H_
