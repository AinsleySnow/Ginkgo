#ifndef _SCOPE_H_
#define _SCOPE_H_

#include "ast/Expr.h"
#include "visitast/Identifier.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class Register;


class Scope
{
public:
    enum class ScopeType { func, file, block, proto };

    Scope(ScopeType s) : scopetype_(s) {}

    ScopeType GetScopeType() const { return scopetype_; }

    const Object* GetObject(const std::string&) const;
    const Func* GetFunc(const std::string&) const;
    const Label* GetLabel(const std::string&) const;
    const Typedef* GetTypedef(const std::string&) const;
    const Member* GetMember(const std::string&) const;

    void AddObject(const std::string&, const CType*, const Register*);
    void AddFunc(const std::string&, const CFuncType*, const Register*);
    void AddLabel(const std::string&);
    void AddTypedef(const std::string&, const CType*);
    void AddMember(const std::string&, const CType*);


private:
    Identifier* FindingHelper(const std::string&, Identifier::IdentType) const;

    std::unordered_multimap<std::string, std::unique_ptr<Identifier>> identmap_{};
    ScopeType scopetype_;
};


class ScopeStack
{
public:
    const Object* SearchObject(const std::string&);
    const Func* SearchFunc(const std::string&);
    const Label* SearchLabel(const std::string&);
    const Typedef* SearchTypedef(const std::string&);
    const Member* SearchMember(const std::string&);

    void PushNewScope(Scope::ScopeType);
    void PopScope();

    Scope& Top();
    Scope& File();

private:
    Scope* filescope_{};
    Scope* blockscope_{};

    std::vector<std::unique_ptr<Scope>> stack_{};
};

#endif // _SCOPE_H_
