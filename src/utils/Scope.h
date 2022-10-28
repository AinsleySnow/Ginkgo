#ifndef _SCOPE_H_
#define _SCOPE_H_

#include "ast/Identifier.h"
#include "visitors/Visitor.h"
#include <map>
#include <memory>
#include <string>
#include <vector>


class Scope
{
public:
    enum class ScopeType { func, file, block, proto };

    Scope(ScopeType s) : scopetype_(s) {}

    Identifier* GetIdentifier(const std::string&) const;
    Identifier* operator[](const std::string&) const;
    void AddIdentifier(std::shared_ptr<Identifier>);

    const Object* GenerateTempVar();
    const Object* GetLastestTempVar(unsigned long) const;
    const Label* GenerateTempLabel();

    const Scope* GetParent() const;

    void Accept(Visitor* v) { v->VisitScope(this); }


private:
    unsigned long templabelindex_{};
    unsigned long tempvarindex_{};

    std::map<std::string, std::shared_ptr<Identifier>> identmap_{};
    std::vector<Scope> scopes_{};
    Scope* parent_{};
    ScopeType scopetype_;
};

#endif // _SCOPE_H_
