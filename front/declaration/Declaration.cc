#include "Declaration.h"

Declaration::Declaration(std::unique_ptr<InitDeclList>& id, std::unique_ptr<DeclSpec>& ds)
{
    initDeclList = std::move(id);
    declSpec = std::move(ds);
}
