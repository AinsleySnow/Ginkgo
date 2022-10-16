#ifndef _SCOPE_H_
#define _SCOPE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "IR.h"
#include "EnumsforEntry.h"
#include "ast/Identifier.h"
#include "types/Type.h"
#include "visitors/Visitor.h"


class Scope
{
public:
    enum class ScopeType { func, file, block, proto };

    Scope(ScopeType s) : scopetype_(s) {}

    const auto GetIdentifier(const std::string&) const;
    void AddIdentifier(std::shared_ptr<Identifier>);
    const auto GetParent() const;

    void Accept(Visitor* v) { v->VisitScope(this); }

private:
    std::map<std::string, std::shared_ptr<Identifier>> identmap_{};
    std::vector<std::shared_ptr<Scope>> scopes_{};
    std::weak_ptr<Scope> parent_{};
    ScopeType scopetype_;

};

#endif // _SCOPE_H_
