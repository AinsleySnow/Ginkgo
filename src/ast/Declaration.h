#ifndef _DECLARATION_H_
#define _DECLARATION_H_

#include "ast/CType.h"
#include "ast/Expr.h"
#include "ast/Statement.h"
#include <list>
#include <memory>
#include <string>
#include <vector>

class Register;
class Visitor;


class Ptr
{
public:
    Ptr() {}
    Ptr(QualType q, std::unique_ptr<Ptr> p) :
        qual_(q), point2_(std::move(p)) {}
    Ptr(QualType q) : qual_(q) {}
    Ptr(std::unique_ptr<Ptr> p) : point2_(std::move(p)) {}

private:
    friend std::unique_ptr<CPtrType> CArithmType::AttachPtr(const Ptr*) const;
    QualType qual_{};
    std::unique_ptr<Ptr> point2_{};
};


class TypeSpec
{
public:
    TypeSpec(Tag t) : spec_(t) {}
    Tag Spec() const { return spec_; }

private:
    friend class IRGen;
    Tag spec_{};
};


class StructUnionSpec : public TypeSpec
{
    // TODO
};

class EnumSpec : public TypeSpec
{
    // TODO
};

class TypedefSpec : public TypeSpec
{
    // TODO
};


class DeclSpec
{
public:
    void Accept(Visitor* v);

    void SetStorage(Tag t) { storagelist_.push_back(t); }
    void SetQual(Tag t) { quallist_.push_back(t); }
    void SetFuncSpec(Tag t) { funcspeclist_.push_back(t); }
    void AddTypeSpec(std::unique_ptr<::TypeSpec> ts);

    auto& Type() { return type_; }
    const CType* Type() const { return type_.get(); }

    TypeTag TypeSpec();
    QualType Qual();
    StorageType Storage();
    FuncSpec Func();

private:
    friend class IRGen;
    bool SetRawSpec(Tag);

    unsigned rawspec_{};

    std::unique_ptr<CType> type_{};
    std::list<std::unique_ptr<::TypeSpec>> speclist_{};
    std::list<Tag> storagelist_{};
    std::list<Tag> quallist_{};
    std::list<Tag> funcspeclist_{};
};


class Declaration
{
public:
    Declaration() {}
    Declaration(const std::string& n) : name_(n) {}

    virtual void Accept(Visitor*) {}

    std::string Name() const { return name_; }

    auto& Type() { return type_; }
    const CType* RawType() const { return type_.get(); }

    const Ptr* GetRawPtr() const { return ptr_.get(); }
    auto& GetPtr() { return ptr_; }

    DeclSpec* GetDeclSpec() const { return declspec_.get(); }
    virtual void SetDeclSpec(std::shared_ptr<::DeclSpec> ds)
    { declspec_ = std::move(ds); }

protected:
    std::shared_ptr<DeclSpec> declspec_{};
    std::unique_ptr<Ptr> ptr_{};
    std::unique_ptr<CType> type_{};
    std::string name_{};
};


class ParamList : public Declaration
{
public:
    void Accept(Visitor* v) override;

    bool& Variadic() { return variadic_; }
    bool Variadic() const { return variadic_; }

    void Append(std::unique_ptr<Declaration> decl);
    void AppendType(const CType* ty);

    const auto& GetParamList() { return paramlist_; }
    const auto& GetParamType() { return paramtype_; }

private:
    friend class IRGen;
    bool variadic_{};
    std::vector<const CType*> paramtype_{};
    std::vector<std::unique_ptr<Declaration>> paramlist_{};
};


struct InitDecl
{
    std::unique_ptr<Declaration> declarator_{};
    std::unique_ptr<Expr> initalizer_{};
    const Register* base_{};
};

class DeclList : public Declaration
{
public:
    void Accept(Visitor* v) override;
    void Append(std::unique_ptr<InitDecl> decl);

    auto begin() { return decllist_.begin(); }
    auto end() { return decllist_.end(); }

    void SetDeclSpec(std::shared_ptr<::DeclSpec>) override;

private:
    std::vector<std::unique_ptr<InitDecl>> decllist_{};
};


class ObjDef : public Declaration
{
public:
    ObjDef() {}
    ObjDef(const std::string& n) : Declaration(n) {}

    void Accept(Visitor* v);

    friend class IRGen;
};


class FuncDef : public Declaration
{
public:
    FuncDef(const std::string& n) :
        Declaration(n), paramlist_(std::make_unique<ParamList>()) {}
    FuncDef(const std::string& n, std::unique_ptr<ParamList> p) :
        Declaration(n), paramlist_(std::move(p)) {}

    void Accept(Visitor* v);

    const auto& GetParamList() { return paramlist_->GetParamList(); }
    const auto& GetParamType() { return paramlist_->GetParamType(); }

    void SetCompound(std::unique_ptr<CompoundStmt> c) { compound_ = std::move(c); }

private:
    friend class IRGen;
    std::unique_ptr<ParamList> paramlist_{};
    std::unique_ptr<CType> return_{};
    std::unique_ptr<CompoundStmt> compound_{};
};


#endif // _DECLARATION_H_
