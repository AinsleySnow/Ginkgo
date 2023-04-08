#include "visitast/TypeBuilder.h"
#include "visitast/Scope.h"
#include "ast/Declaration.h"


void TypeBuilder::VisitArrayDef(ArrayDef* def)
{
    if (def->Size())
        def->Size()->Accept(&visitor_);
    def->Child()->Accept(&visitor_);

    std::unique_ptr<CArrayType> array = nullptr;

    if (def->Size())
    {
        size_t size = 0;
        if (def->Size()->IsConstant())
        {
            auto intconst = static_cast<
                const IntConst*>(def->Size()->Val());
            size = intconst->Val();
        }
        array = std::make_unique<CArrayType>(
            std::move(def->Child()->Type()), size);
    }
    else
        array = std::make_unique<CArrayType>(
            std::move(def->Child()->Type()));

    array->VariableLen() = def->Variable();
    array->Qual() = def->Qual();
    array->Static() = def->Static();

    def->Type() = std::move(array);
}


std::unique_ptr<CEnumType> TypeBuilder::EnumHelper(const EnumSpec* spec)
{
    // empty; the enum must have been defined somewhere
    if (!spec->EnumeratorList())
    {
        auto enumty = scopestack_.SearchCustomed(spec->Name());
        auto enumcty = enumty->GetCType();
        return std::make_unique<CEnumType>(
            *static_cast<const CEnumType*>(enumcty));
    }

    // not empty; we're declaring a new enum
    std::shared_ptr<CType> underlying = nullptr;
    if (spec->EnumeratorType())
    {
        spec->EnumeratorType()->Accept(&visitor_);
        underlying = std::move(spec->EnumeratorType()->Type());
    }
    // if no underlying type is specified, postpone the
    // decision until all enum constants have been seen

    // the underlying type will be placed in type_ 
    // field inherited from Expr class
    spec->EnumeratorList()->Accept(&visitor_);
    if (!underlying)
        underlying = std::move(spec->EnumeratorList()->Type());

    auto ty = std::make_unique<CEnumType>(spec->Name(), underlying);
    ty->Reserve(spec->EnumeratorList()->Count());

    for (auto& mem : *spec->EnumeratorList())
    {        
        auto pmem = scopestack_.Top().AddMember(mem->Name(), ty.get(),
            static_cast<const IntConst*>(mem->Val()));        
        ty->AddMember(pmem);
    }

    return std::move(ty);
}

void TypeBuilder::VisitDeclSpec(DeclSpec* spec)
{
    auto tag = spec->GetTypeTag();

    if (tag == TypeTag::_void)
        spec->Type() = std::make_unique<CVoidType>();
    else if (int(tag) & int(TypeTag::scalar))
    {
        spec->Type() = std::make_unique<CArithmType>(tag);
        spec->Type()->Qual() = spec->Qual();
        spec->Type()->Storage() = spec->Storage();
    }
    else if (tag == TypeTag::_enum)
    {
        auto ty = EnumHelper(spec->GetEnumSpec());
        ty->Qual() = spec->Qual();
        ty->Storage() = spec->Storage();

        scopestack_.Top().AddCustomed(ty->Name(), ty.get());
        spec->Type() = std::move(ty);
    }
}


void TypeBuilder::VisitFuncDef(FuncDef* def)
{
    def->Child()->Accept(&visitor_);
    def->paramlist_->Accept(&visitor_);

    auto funccty = std::make_unique<CFuncType>(
        std::move(def->Child()->Type()),
        def->GetParamList().size());

    DeclSpec* declspec = def->Child()->InnerMost()->ToDeclSpec();

    bool _inline = declspec->Func().IsInline();
    bool _noreturn = declspec->Func().IsNoreturn();
    funccty->Inline() = _inline;
    funccty->Noreturn() = _noreturn;

    for (auto param : def->GetParamType())
        funccty->AddParam(std::move(param));

    def->Type() = std::move(funccty);
}


void TypeBuilder::VisitObjDef(ObjDef* def)
{
    def->Child()->Accept(&visitor_);
    def->Type() = std::move(def->Child()->Type());
}


void TypeBuilder::VisitPtrDef(PtrDef* def)
{
    def->Child()->Accept(&visitor_);
    def->Type() = std::make_unique<CPtrType>(
        std::move(def->Child()->Type()));
    def->Type()->Qual() = def->qual_;
}
