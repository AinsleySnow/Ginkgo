#include "Declaration.h"

Declaration::Declaration(std::unique_ptr<DeclSpec>&& ds, std::unique_ptr<InitDeclList>&& id)
{
    initDeclList = std::move(id);
    declSpec = std::move(ds);
}
