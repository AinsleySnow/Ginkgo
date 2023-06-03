#ifndef _DECLARATION_H_
#define _DECLARATION_H_

#include "ast/CType.h"
#include "ast/Expr.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "visitast/Scope.h"
#include <list>
#include <memory>
#include <string>
#include <vector>

class DeclSpec;
class ObjDef;
class FuncDef;
class Register;
class ASTVisitor;


class Declaration
{
public:
    virtual void Accept(ASTVisitor*) {}

    auto& Type() { return type_; }
    const CType* RawType() const { return type_.get(); }

    virtual bool IsDeclSpec() const { return false; }
    virtual bool IsObjDef() const { return false; }
    virtual bool IsFuncDef() const { return false; }
    virtual bool IsHeterList() const { return false; }
    virtual DeclSpec* ToDeclSpec() { return nullptr; }
    virtual ObjDef* ToObjDef() { return nullptr; }
    virtual FuncDef* ToFuncDef() { return nullptr; }
    virtual HeterList* ToHeterList() { return nullptr; }
    virtual const DeclSpec* ToDeclSpec() const { return nullptr; }
    virtual const ObjDef* ToObjDef() const { return nullptr; }
    virtual const FuncDef* ToFuncDef() const { return nullptr; }
    virtual const HeterList* ToHeterList() const { return nullptr; }

    Declaration* Child() { return child_.get(); }
    const Declaration* Child() const { return child_.get(); }
    void SetChild(std::shared_ptr<Declaration> chd) { child_ = chd; }

    Declaration* InnerMost();

protected:
    std::shared_ptr<Declaration> child_{};
    std::unique_ptr<CType> type_{};
};


class TypeSpec
{
public:
    TypeSpec(Tag t) : spec_(t) {}
    Tag Spec() const { return spec_; }

private:
    Tag spec_{};
};


using HeterFields = std::vector<std::unique_ptr<Declaration>>;

class HeterSpec : public TypeSpec
{
public:
    HeterSpec(Tag t) : TypeSpec(t) {}
    HeterSpec(Tag t, const std::string& n) : TypeSpec(t), name_(n) {}

    void LoadHeterFields(HeterFields&& f);
    const auto& GetHeterFields() const { return fields_; }

    auto&& GetScope() { return std::move(scope_); }
    const auto& GetScope() const { return scope_; }
    void LoadScope(std::unique_ptr<Scope> s) { scope_ = std::move(s); }

    std::string Name() const { return name_; }

private:
    std::unique_ptr<Scope> scope_{
        std::make_unique<Scope>(Scope::ScopeType::block) };

    std::string name_{};
    HeterFields fields_{};
};


class EnumSpec : public TypeSpec
{
public:
    EnumSpec(std::unique_ptr<EnumList> el,
        std::unique_ptr<Declaration> ty = nullptr) :
        TypeSpec(Tag::_enum), enumlist_(std::move(el)),
        enumspec_(std::move(ty)) {}
    EnumSpec(const std::string& n,
        std::unique_ptr<Declaration> ty = nullptr) :
        TypeSpec(Tag::_enum), name_(n),
        enumspec_(std::move(ty)) {}
    EnumSpec(const std::string& n,
        std::unique_ptr<EnumList> el,
        std::unique_ptr<Declaration> ty = nullptr) :
        TypeSpec(Tag::_enum),
        name_(n), enumlist_(std::move(el)),
        enumspec_(std::move(ty)) {}

    const auto& Name() const { return name_; }
    const auto& EnumeratorType() const { return enumspec_; }
    const auto& EnumeratorList() const { return enumlist_; }

private:
    std::string name_{};
    std::unique_ptr<Declaration> enumspec_{};
    std::unique_ptr<EnumList> enumlist_{};
};

class TypedefSpec : public TypeSpec
{
    // TODO
};


class DeclSpec : public Declaration
{
public:
    void Accept(ASTVisitor* v) override;

    bool IsDeclSpec() const override { return true; }
    DeclSpec* ToDeclSpec() override { return this; }
    const DeclSpec* ToDeclSpec() const override { return this; }

    void SetStorage(Tag t) { storagelist_.push_back(t); }
    void SetQual(Tag t) { quallist_.push_back(t); }
    void SetFuncSpec(Tag t) { funcspeclist_.push_back(t); }

    void AddTypeSpec(std::unique_ptr<::TypeSpec> ts);
    const EnumSpec* GetEnumSpec() const;
    const HeterSpec* GetHeterSpec() const;

    TypeTag GetTypeTag();
    QualType Qual();
    StorageType Storage();
    FuncSpec Func();

private:
    bool SetRawSpec(Tag);

    unsigned rawspec_{};

    std::list<std::unique_ptr<::TypeSpec>> speclist_{};
    std::list<Tag> storagelist_{};
    std::list<Tag> quallist_{};
    std::list<Tag> funcspeclist_{};
};


class HeterList : public Declaration
{
public:
    void Accept(ASTVisitor* v) override;

    bool IsHeterList() const override { return true; }
    HeterList* ToHeterList() override { return this; }
    const HeterList* ToHeterList() const override { return this; }

    void Append(std::unique_ptr<Declaration>);
    auto begin() { return decllist_.begin(); }
    auto end() { return decllist_.end(); }

    auto& InScope() { return inscope_; }
    const Scope* InScope() const { return inscope_; }

private:
    Scope* inscope_{};
    std::vector<std::unique_ptr<Declaration>> decllist_{};
};


class ParamList : public Declaration
{
public:
    void Accept(ASTVisitor* v) override;

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
    void Accept(ASTVisitor* v) override;
    void Append(std::unique_ptr<InitDecl> decl);

    auto begin() { return decllist_.begin(); }
    auto end() { return decllist_.end(); }

private:
    std::vector<std::unique_ptr<InitDecl>> decllist_{};
};


class ObjDef : public Declaration
{
public:
    ObjDef() {}
    ObjDef(const std::string& n) : name_(n) {}

    void Accept(ASTVisitor* v);

    bool IsObjDef() const override { return true; }
    ObjDef* ToObjDef() override { return this; }
    const ObjDef* ToObjDef() const override { return this; }

    std::string Name() { return name_; }
    void SetCompound(std::unique_ptr<CompoundStmt> c) { compound_ = std::move(c); }

private:
    friend class IRGen;
    std::string name_{};
    std::unique_ptr<CompoundStmt> compound_{};
};


class PtrDef : public Declaration
{
public:
    PtrDef() {}
    PtrDef(QualType q, std::shared_ptr<Declaration> p) :
        qual_(q) { child_ = p; }
    PtrDef(QualType q) : qual_(q) {}
    PtrDef(std::shared_ptr<Declaration> p) { child_ = p; }

    void Accept(ASTVisitor*) override;

private:
    friend class IRGen;
    friend class TypeBuilder;
    QualType qual_{};
};


class ArrayDef : public Declaration
{
public:
    ArrayDef() {}
    ArrayDef(std::unique_ptr<Expr> size) :
        size_(std::move(size)) {}

    void Accept(ASTVisitor*);

    bool Variable() const { return variable_; }
    bool& Variable() { return variable_; }
    bool Static() const { return static_; }
    bool& Static() { return static_; }
    auto Qual() const { return qual_; }
    auto& Qual() { return qual_; }

    auto& Size() { return size_; }
    const auto& Size() const { return size_; }

private:
    bool variable_{};
    bool static_{};
    QualType qual_{};
    std::unique_ptr<Expr> size_{};
};


class FuncDef : public Declaration
{
public:
    FuncDef() : paramlist_(std::make_unique<ParamList>()) {}
    FuncDef(std::unique_ptr<ParamList> p) : paramlist_(std::move(p)) {}

    void Accept(ASTVisitor* v);

    bool IsFuncDef() const override { return true; }
    FuncDef* ToFuncDef() override { return this; }
    const FuncDef* ToFuncDef() const override { return this; }

    const auto& GetParamList() { return paramlist_->GetParamList(); }
    const auto& GetParamType() { return paramlist_->GetParamType(); }

private:
    friend class IRGen;
    friend class TypeBuilder;
    std::unique_ptr<ParamList> paramlist_{};
};


class BitFieldDef : public Declaration
{
public:
    BitFieldDef(std::unique_ptr<Expr> e) : expr_(std::move(e)) {}
    BitFieldDef(std::unique_ptr<Declaration> d, std::unique_ptr<Expr> e) :
        decl_(std::move(d)), expr_(std::move(e)) {}

private:
    std::unique_ptr<Declaration> decl_{};
    std::unique_ptr<Expr> expr_{};
};


#endif // _DECLARATION_H_
