#include "visitast/TypeBuilder.h"
#include "visitast/Scope.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"
#include <climits>
#include <cfloat>


std::shared_ptr<CType> TypeBuilder::EnlargeCType(std::shared_ptr<CType> ty, int ext)
{
    auto arithm = ty->As<CArithmType>();
    auto newsize = arithm->Size() + ext;
    if (arithm->IsUnsigned())
    {
        if (newsize <= 2)
            return std::make_shared<CArithmType>(TypeTag::uint16);
        else if (newsize > 3 && newsize <= 4)
            return std::make_shared<CArithmType>(TypeTag::uint32);
        else if (newsize > 4 && newsize <= 8)
            return std::make_shared<CArithmType>(TypeTag::uint64);
    }
    else
    {
        if (newsize <= 2)
            return std::make_shared<CArithmType>(TypeTag::int16);
        else if (newsize > 3 && newsize <= 4)
            return std::make_shared<CArithmType>(TypeTag::int32);
        else if (newsize > 4 && newsize <= 8)
            return std::make_shared<CArithmType>(TypeTag::int64);
    }

    return nullptr;
}


std::shared_ptr<CType> TypeBuilder::GetCTypeByValue(
    std::shared_ptr<CType> lhs, std::shared_ptr<CType> rhs, uint64_t value)
{
    auto bigger = MatchCType(lhs, rhs);
    int size = bigger->Size() * 8;
    int pos = 64 - __builtin_clzll(value | 1);

    // enlarge the type if it can't hold the value
    if (pos > size)
    {
        int extend = (pos - size) % 8 + 1;
        bigger = EnlargeCType(bigger, extend);
    }

    return bigger;
}

std::shared_ptr<CType> TypeBuilder::GetCTypeByValue(
    std::shared_ptr<CType> lhs, std::shared_ptr<CType> rhs, double value)
{
    auto bigger = MatchCType(lhs, rhs);

    if (bigger->Size() == 4 && (value > FLT_MAX || value < FLT_MIN))
        bigger = std::make_shared<CArithmType>(TypeTag::flt64);

    return bigger;
}


std::shared_ptr<CType> TypeBuilder::MatchCType(
    std::shared_ptr<CType> lhs, std::shared_ptr<CType> rhs)
{
    auto la = lhs->As<CArithmType>();
    auto ra = rhs->As<CArithmType>();

    if (la->IsFloat() && ra->IsInteger()) return lhs;
    else if (la->IsInteger() && ra->IsFloat()) return rhs;
    else if (la->operator>(*ra)) return lhs;
    else if (la->operator<(*ra)) return rhs;

    // the two types are compatible
    if (la->IsFloat() && ra->IsFloat()) return lhs;
    if ((la->IsUnsigned() && ra->IsUnsigned()) ||
        (!la->IsUnsigned() && !ra->IsUnsigned())) return lhs;
    return la->IsUnsigned() ? lhs : rhs;
}


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
        auto pmem = scopestack_.Top().AddMember(
            mem->Name(), ty.get(), mem->Val()->As<IntConst>());        
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


void TypeBuilder::VisitArrayExpr(ArrayExpr* expr)
{
    if (expr->Type()) return;
    expr->Identifier()->Accept(this);

    auto ty = expr->Identifier()->Type();
    if (ty->Is<CArrayType>())
    {
        auto array = ty->As<CArrayType>();
        expr->Type() = std::move(array->ArrayOf()->Clone());
    }
    else if (ty->Is<CPtrType>())
    {
        auto ptr = ty->As<CPtrType>();
        expr->Type() = std::move(ptr->Point2()->Clone());
    }
}


void TypeBuilder::VisitAssignExpr(AssignExpr* expr)
{
    if (expr->Type()) return;
    expr->Left()->Accept(this);
    expr->Right()->Accept(this);

    auto lhs = expr->Left()->Type();
    auto rhs = expr->Right()->Type();
    expr->Type() = MatchCType(lhs, rhs);
}


void TypeBuilder::VisitBinaryExpr(BinaryExpr* expr)
{
    if (expr->Type()) return;
    expr->Left()->Accept(this);
    expr->Right()->Accept(this);

    // the value is calculated by constant folding
    if (expr->IsConstant())
    {
        if (expr->Val()->Is<IntConst>())
            expr->Type() = GetCTypeByValue(
                expr->Left()->Type(), expr->Right()->Type(), expr->Val()->As<IntConst>()->Val());
        else if (expr->Val()->Is<FloatConst>())
            expr->Type() = GetCTypeByValue(
                expr->Left()->Type(), expr->Right()->Type(), expr->Val()->As<FloatConst>()->Val());
        return;
    }

    auto lhs = expr->Left()->Type();
    auto rhs = expr->Right()->Type();
    expr->Type() = MatchCType(lhs, rhs);
}


void TypeBuilder::VisitCallExpr(CallExpr* expr)
{
    if (expr->Type()) return;

    auto func = expr->Postfix()->Type();
    if (func->Is<CFuncType>())
    {
        auto cfunc = func->As<CFuncType>();
        expr->Type() = std::move(cfunc->ReturnType()->Clone());
    }
    else if (func->Is<CPtrType>())
    {
        auto ptr = func->As<CPtrType>();
        auto cfunc = ptr->Point2()->As<CFuncType>();
        expr->Type() = std::move(cfunc->ReturnType()->Clone());
    }
}


void TypeBuilder::VisitCastExpr(CastExpr* expr)
{
    if (expr->Type()) return;
    expr->TypeName()->Accept(this);
    expr->Type() = std::move(expr->TypeName()->Type());
}

void TypeBuilder::VisitCondExpr(CondExpr* expr)
{
    if (expr->Type()) return;
    expr->TrueExpr()->Accept(this);
    expr->Type() = expr->TrueExpr()->Type();
}

void TypeBuilder::VisitDataofExpr(DataofExpr* expr)
{
    if (expr->Type()) return;
    expr->Type() = std::make_unique<CArithmType>(TypeTag::int32);
}

void TypeBuilder::VisitEnumConst(EnumConst* expr)
{
    if (expr->Type()) return;

    // no need to build recursively here.
    // ValueExpr in EnumConst always has a type.
    expr->Type() = expr->ValueExpr()->Type();
}


void TypeBuilder::VisitEnumList(EnumList* list)
{
    if (list->Type()) return;

    list->Type() = list->front()->Type();
    for (auto& mem : *list)
    {
        auto raw = mem->Type()->As<CArithmType>();
        auto old = list->Type()->As<CArithmType>();
        if (raw->operator>(*old))
        {
            // if one is unsigned and the other is signed
            if (raw->IsUnsigned() ^ old->IsUnsigned())
                list->Type() = MatchCType(mem->Type(), list->Type());
            else
                list->Type() = mem->Type();
        }
        else if (raw->Compatible(*old) && (raw->IsUnsigned() ^ old->IsUnsigned()))
            list->Type() = MatchCType(mem->Type(), list->Type());
    }
}


void TypeBuilder::VisitIdentExpr(IdentExpr* expr)
{
    if (expr->Type()) return;
    auto decl = scopestack_.SearchObject(expr->Name());
    expr->Type() = std::move(decl->GetCType()->Clone());
}


void TypeBuilder::VisitLogicalExpr(LogicalExpr* expr)
{
    if (expr->Type()) return;
    expr->Type() = std::make_unique<CArithmType>(TypeTag::int32);
}


void TypeBuilder::VisitStrExpr(StrExpr* str)
{
    if (str->Type()) return;

    int width = 0;
    auto setwidth = [&width] (int w) {
        if (width) width = -1;
        width = w;
    };

    int i = 0;
    while ((i = str->Content().
        find_first_of('"', i)) != std::string::npos)
    {
        if (i == 0) continue;

        char prefix = str->Content()[i - 1];
        if (prefix == '8') setwidth(1);
        else if (prefix == 'u') setwidth(2);
        else if (prefix == 'U' || prefix == 'L') setwidth(4);
    
        // now i is inside a string
        while ((i = str->Content().
            find_first_of('"', i)) != std::string::npos)
            if (str->Content()[i - 1] != '\\')
                break;
        
        i += 1;
    }

    str->Width() = width <= 0 ? 1 : width;

    // Differently-perfixed wide string will be treated as
    // a character string literal. (6.4.5[5] in the C23 standard)
    if (width == 0 || width == -1)
        str->Type() = std::make_unique<CPtrType>(
            std::make_unique<CArithmType>(TypeTag::int8));
    else if (width == 1)
        str->Type() = std::make_unique<CPtrType>(
            std::make_unique<CArithmType>(TypeTag::uint8));
    else if (width == 2)
        str->Type() = std::make_unique<CPtrType>(
            std::make_unique<CArithmType>(TypeTag::uint16));
    else if (width == 4)
        str->Type() = std::make_unique<CPtrType>(
            std::make_unique<CArithmType>(TypeTag::uint32));
}


void TypeBuilder::VisitUnaryExpr(UnaryExpr* expr)
{
    if (expr->Type()) return;

    expr->Content()->Accept(this);
    if (expr->Op() == Tag::asterisk)
    {
        auto ptr = expr->Content()->Type()->As<CPtrType>();
        expr->Type() = ptr->Point2()->Clone();
    }
    else if (expr->Op() == Tag::_and)
    {
        auto ty = expr->Content()->Type()->Clone();
        expr->Type() = std::make_unique<CPtrType>(std::move(ty));
    }
    else
        expr->Type() = expr->Content()->Type();
}
