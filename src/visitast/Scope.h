#ifndef _SCOPE_H_
#define _SCOPE_H_

#include "ast/Expr.h"
#include "visitast/Identifier.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class IntConst;
class Register;


class Scope
{
public:
    enum class ScopeType { func, file, block, proto };

    Scope(ScopeType s) : scopetype_(s) {}

    ScopeType GetScopeType() const { return scopetype_; }
    void Extend(const Scope&);

    auto begin() { return identmap_.begin(); }
    auto end() { return identmap_.end(); }

    const Object* GetObject(const std::string&) const;
    const Func* GetFunc(const std::string&) const;
    const Label* GetLabel(const std::string&) const;
    const Typedef* GetTypedef(const std::string&) const;
    const CustomedType* GetCustomed(const std::string&) const;
    const Member* GetMember(const std::string&) const;

    Object* AddObject(const std::string&, const CType*, const Register*);
    Func* AddFunc(const std::string&, const CFuncType*, const Register*);
    Label* AddLabel(const std::string&);
    Typedef* AddTypedef(const std::string&, const CType*);
    CustomedType* AddCustomed(const std::string&, const CType*);
    Member* AddMember(const std::string&, const CType*, const IntConst* = nullptr);


private:
    Identifier* FindingHelper(const std::string&, Identifier::IdentType) const;

    std::map<std::string, std::unique_ptr<Identifier>> identmap_{};
    ScopeType scopetype_;
};


class ScopeStack
{
public:
    const Object* SearchObject(const std::string&);
    const Func* SearchFunc(const std::string&);
    const Label* SearchLabel(const std::string&);
    const CType* UnderlyingTydef(const std::string&);
    const CustomedType* SearchCustomed(const std::string&);
    const Member* SearchMember(const std::string&);

    void PushNewScope(Scope::ScopeType);
    void PopScope();
    void LoadNewScope(std::unique_ptr<Scope>);
    std::unique_ptr<Scope> RestoreScope();

    Scope& Top();
    Scope& File();

private:
    Scope* filescope_{};
    Scope* funcscope_{};

    std::vector<std::unique_ptr<Scope>> stack_{};
};

#endif // _SCOPE_H_
