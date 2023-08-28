#include "visitast/TypeBuilder.h"
#include "visitast/Scope.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"
#include "IR/IROperand.h"
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


static int MatchCArithmType(const CType* lhs, const CType* rhs)
{
    auto la = lhs->As<CArithmType>();
    auto ra = rhs->As<CArithmType>();

    if (la->IsFloat() && ra->IsInteger()) return 0;
    else if (la->IsInteger() && ra->IsFloat()) return 1;
    else if (la->operator>(*ra)) return 0;
    else if (la->operator<(*ra)) return 1;

    // the two types are compatible
    if (la->IsFloat() && ra->IsFloat()) return 0;
    if ((la->IsUnsigned() && ra->IsUnsigned()) ||
        (!la->IsUnsigned() && !ra->IsUnsigned())) return 0;
    return la->IsUnsigned() ? 0 : 1;
}

std::shared_ptr<CType> TypeBuilder::MatchCType(
    std::shared_ptr<CType> lhs, std::shared_ptr<CType> rhs)
{
    auto matchedarithm = [&lhs, &rhs] (const CType* l, const CType* r) {
        return MatchCArithmType(l, r) == 0 ? lhs : rhs;
    };

    // for things like int* + 2, int* = 2
    if (lhs->Is<CPtrType>() && rhs->Is<CArithmType>())
        return lhs;
    else if (lhs->Is<CArithmType>() && rhs->Is<CPtrType>())
        return rhs;
    // Both lhs and rhs are pointers? Then we are handling
    // ptr = ptr or ptr - ptr. Note that while ptr - ptr is okay,
    // ptr + ptr is illegal in C and this sort of error
    // should be recognized in CodeChk. The type of the ptrs,
    // given minus as the operator, are bound to be same
    // as guaranteed by CodeChk.
    else if (lhs->Is<CPtrType>() && rhs->Is<CPtrType>())
        return lhs;
    // for things like ArrayName = 2, ArrayName[2]
    else if (lhs->Is<CArrayType>() && rhs->Is<CArithmType>())
        return std::make_unique<CPtrType>(lhs->As<CArrayType>()->ArrayOf()->Clone());
    else if (lhs->Is<CArithmType>() && rhs->Is<CArrayType>())
        return std::make_unique<CPtrType>(rhs->As<CArrayType>()->ArrayOf()->Clone());
    // for things like FuncPtr = FuncName
    else if (lhs->Is<CPtrType>() && rhs->Is<CFuncType>())
        return lhs;
    else if (lhs->Is<CFuncType>() && rhs->Is<CPtrType>())
        return rhs;
    // for things like PtrName = ArrayName
    else if (lhs->Is<CPtrType>() && rhs->Is<CArrayType>())
        return lhs;
    // What about the variables with enum type?
    else if (lhs->Is<CEnumType>() && rhs->Is<CEnumType>())
    {
        auto left = lhs->As<CEnumType>()->Underlying();
        auto right = rhs->As<CEnumType>()->Underlying();
        return matchedarithm(left, right);
    }
    else if (lhs->Is<CEnumType>())
    {
        auto left = lhs->As<CEnumType>()->Underlying();
        return matchedarithm(left, rhs.get());
    }
    else if (rhs->Is<CEnumType>())
    {
        auto right = rhs->As<CEnumType>()->Underlying();
        return matchedarithm(lhs.get(), right);
    }

    return matchedarithm(lhs.get(), rhs.get());
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
            auto intconst = static_cast<const IntConst*>(def->Size()->Val());
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
    array->Storage() = array->ArrayOf()->Storage();
    array->Static() = def->Static();

    def->Type() = std::move(array);
}


std::unique_ptr<CEnumType> TypeBuilder::EnumHelper(const EnumSpec* spec, size_t align)
{
    // empty; the enum must have been defined somewhere
    if (!spec->EnumeratorList())
    {
        auto enumty = scopestack_.SearchCustomed(spec->Name());
        auto enumcty = enumty->GetCType();
        auto ty = enumcty->Clone();
        if (align != 0)
            ty->Align() = align;
        return std::unique_ptr<CEnumType>(static_cast<CEnumType*>(ty.release()));
    }

    // not empty; we're declaring a new enum
    std::unique_ptr<CType> underlying = nullptr;
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
        underlying = std::move(spec->EnumeratorList()->Type()->Clone());

    std::unique_ptr<CEnumType> ty = nullptr;
    if (align > underlying->Align())
        ty = std::make_unique<CEnumType>(spec->Name(), std::move(underlying), align);
    else
        ty = std::make_unique<CEnumType>(spec->Name(), std::move(underlying));
    ty->Reserve(spec->EnumeratorList()->Count());

    for (auto& mem : *spec->EnumeratorList())
    {        
        auto pmem = scopestack_.Top().AddMember(
            mem->Name(), ty.get(), mem->Val()->As<IntConst>());        
        ty->AddMember(pmem);
    }

    return std::move(ty);
}

template <class T>
std::unique_ptr<T> TypeBuilder::HeterHelper(const HeterSpec* spec, size_t align)
{
    auto& fields = spec->GetHeterFields();

    // if the struct is declared else where
    if (fields.empty())
    {
        auto ty = scopestack_.SearchCustomed(spec->Name())->GetCType()->As<T>();
        auto cty = std::make_unique<T>(*ty);
        if (align)
            cty->Align() = align;
        return std::move(cty);
    }

    int fieldindex = 0;
    auto ty = std::make_unique<T>();
    scopestack_.LoadNewScope(const_cast<HeterSpec*>(spec)->GetScope());
    for (auto& decl : fields)
    {
        decl->Accept(this);
        // decl is either a HeterList or a DeclSpec, as guaranteed in yacc actions.
        // If decl is HeterList, then we generate type and add things to the
        // scope in the VisitHeterList method. Otherwise, if type of decl is
        // a new struct or union, then we add the type to the scope stack;
        // if decl is a anonymous type, copy all the fields in its scope to
        // the current scope.
        if (decl->IsHeterList())
        {
            for (auto& var : *decl->ToHeterList())
            {
                if constexpr (std::is_same_v<T, CStructType>)
                    ty->AddStructMember(
                        var->ToObjDef()->Name(), var->RawType(), false, fieldindex++);
                else
                    ty->AddUnionMember(
                        var->ToObjDef()->Name(), var->RawType(), false, fieldindex++);
            }
        }
        else if (decl->Type()->Is<CHeterType>())
        {
            // Hmm, since now decl must be DeclSpec...
            auto hs = decl->ToDeclSpec()->GetHeterSpec();
            // not anoymous; let it be
            if (!hs->Name().empty())
            {
                scopestack_.Top().AddCustomed(hs->Name(), decl->RawType());
                continue;
            }
            // merge two scope
            scopestack_.Top().Extend(*hs->GetScope());
            // and then add members to ty
            for (auto& [n, i] : *hs->GetScope())
            {
                if (i->GetIdentType() == Identifier::IdentType::member)
                {
                    if constexpr (std::is_same_v<T, CStructType>)
                        ty->AddStructMember(n, decl->RawType(), true, fieldindex);
                    else
                        ty->AddUnionMember(n, decl->RawType(), true, fieldindex);
                }
            }
            fieldindex += 1;
        }
    }
    const_cast<HeterSpec*>(spec)->LoadScope(std::move(scopestack_.RestoreScope()));

    if (!spec->Name().empty())
        scopestack_.Top().AddCustomed(spec->Name(), ty.get());
    if (align > ty->Align())
        ty->Align() = align;
    return std::move(ty);
}

std::unique_ptr<CType> TypeBuilder::TypedefHelper(const TypedefSpec* spec, size_t align)
{
    auto t = scopestack_.UnderlyingTydef(spec->Name());
    auto ty = t->Clone();
    ty->Storage().UnsetToken(StorageTag::_typedef);
    if (align > ty->Align())
        ty->Align() = align;
    return std::move(ty);
}


void TypeBuilder::VisitDeclSpec(DeclSpec* spec)
{
    auto& aligns = spec->AlignSpecs();
    size_t align = 0;
    auto isdecl = [] (const AlignSpec& as) {
        return std::holds_alternative<std::unique_ptr<Declaration>>(as);
    };
    for (auto& as : aligns)
    {
        if (isdecl(as))
        {
            auto& decl = std::get<0>(as);
            decl->Accept(this);
            if (decl->Type()->Align() > align)
                align = decl->Type()->Align();
        }
        else // if isexpr(as)
        {
            auto& expr = std::get<1>(as);
            expr->Accept(&visitor_);
            auto val = expr->Val()->As<IntConst>()->Val();
            if (val > align)
                align = val;
        }
    }

    auto tag = spec->GetTypeTag();

    if (tag == TypeTag::_void)
        spec->Type() = std::make_unique<CVoidType>();
    else if (int(tag) & int(TypeTag::scalar))
    {
        spec->Type() = std::make_unique<CArithmType>(tag, align);
        spec->Type()->Qual() = spec->Qual();
        spec->Type()->Storage() = spec->Storage();
    }
    else if (tag == TypeTag::_enum)
    {
        auto ty = EnumHelper(spec->GetEnumSpec(), align);
        ty->Qual() = spec->Qual();
        ty->Storage() = spec->Storage();

        scopestack_.Top().AddCustomed(ty->Name(), ty.get());
        spec->Type() = std::move(ty);
    }
    else if (tag == TypeTag::_struct)
    {
        auto ty = HeterHelper<CStructType>(spec->GetHeterSpec(), align);
        ty->Qual() = spec->Qual();
        ty->Storage() = spec->Storage();

        scopestack_.Top().AddCustomed(ty->Name(), ty.get());
        spec->Type() = std::move(ty);
    }
    else if (tag == TypeTag::_union)
    {
        auto ty = HeterHelper<CUnionType>(spec->GetHeterSpec(), align);
        ty->Qual() = spec->Qual();
        ty->Storage() = spec->Storage();

        scopestack_.Top().AddCustomed(ty->Name(), ty.get());
        spec->Type() = std::move(ty);
    }
    else if (tag == TypeTag::_typedef)
    {
        auto ty = TypedefHelper(spec->GetTypedefSpec(), align);
        ty->Qual() = spec->Qual();
        ty->Storage() = spec->Storage();
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

    funccty->Storage() = declspec->Storage();
    funccty->Qual() = declspec->Qual();
    funccty->Inline() = declspec->Func().IsInline();
    funccty->Noreturn() = declspec->Func().IsNoreturn();
    funccty->Variadic() = def->paramlist_->Variadic();

    for (auto param : def->GetParamType())
        funccty->AddParam(param);

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
    auto ty = std::make_unique<CPtrType>(std::move(def->Child()->Type()));
    ty->Qual() = def->qual_;
    ty->Storage() = ty->Point2()->Storage();
    def->Type() = std::move(ty);
}


void TypeBuilder::VisitHeterList(HeterList* list)
{
    for (auto& d : *list)
    {
        d->Accept(this);
        list->InScope()->AddMember(
            d->ToObjDef()->Name(), d->RawType());
    }
}


void TypeBuilder::VisitAccessExpr(AccessExpr* expr)
{
    if (expr->Type())
        return;
    expr->Postfix()->Accept(this);
    const auto& field = expr->Field();
    int index = 0;

    const CHeterType* heterty = nullptr;
    if (expr->Op() == Tag::arrow)
        heterty = expr->Postfix()->Type()->
            As<CPtrType>()->Point2()->As<CHeterType>();
    else // if (expr->Op() == Tag::dot)
        heterty = expr->Postfix()->Type()->As<CHeterType>();

    while (true)
    {
        index = (*heterty)[field];
        auto [next, ty, _] = (*heterty)[index];
        if (!next)
        {
            expr->Type() = ty->Clone();
            break;
        }
        heterty = ty->As<CHeterType>();
    }
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

    auto lhs = expr->Left()->Type();
    auto rhs = expr->Right()->Type();
    expr->Type() = MatchCType(lhs, rhs);
}


void TypeBuilder::VisitCallExpr(CallExpr* expr)
{
    if (expr->Type()) return;

    if (expr->Postfix()->IsIdentifier())
    {
        auto name = expr->Postfix()->ToIdentifier()->Name();
        if (name == "__Ginkgo_assert")
        {
            expr->Type() = std::make_unique<CVoidType>();
            return;
        }
    }

    expr->Postfix()->Accept(this);
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

void TypeBuilder::VisitSzAlgnExpr(SzAlgnExpr* expr)
{
    if (expr->Type()) return;
    expr->Type() = std::make_unique<CArithmType>(TypeTag::uint64);
}

void TypeBuilder::VisitEnumConst(EnumConst* expr)
{
    if (expr->Type()) return;

    // no need to build recursively here.
    // ValueExpr in EnumConst always has a type.
    if (expr->ValueExpr())
        expr->Type() = expr->ValueExpr()->Type();
    // what if ValueExpr() is empty?
    else
        expr->Type() = std::make_shared<CArithmType>(TypeTag::int32);
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
    if (decl)
    {
        expr->Type() = std::move(decl->GetCType()->Clone());
        return;
    }

    // then... what if decl denotes a function?
    auto func = scopestack_.SearchFunc(expr->Name());
    expr->Type() = std::move(func->GetCType()->Clone());
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
        if (!width)          width = w;
        else if (width != w) width = -1;
    };

    auto next = [&str] (size_t i) {
        // Assume that str->Content()[i] is always
        // the '"' that marks the start of a string.
        // First navigate to the ending '"'
        while ((i = str->Content().
            find_first_of('"', i + 1)) != std::string::npos)
            if (str->Content()[i - 1] != '\\')
                break;
        // Then get the starting '"' of next string
        i = str->Content().find_first_of('"', i + 1);
        return i;
    };

    bool sign = false;
    for (auto i = str->Content().find_first_of('"'); i != std::string::npos; i = next(i))
    {
        if (i == 0)
            continue;
        char prefix = str->Content()[i - 1];
        if (prefix == '8')      { setwidth(1); }
        else if (prefix == 'u') { setwidth(2); }
        else if (prefix == 'U') { setwidth(4); }
        else if (prefix == 'L') { setwidth(4); sign = true; }
    }

    str->Width() = width <= 0 ? 1 : width;

    // Differently-perfixed wide string will be treated as
    // a character string literal. (6.4.5[5] in the C23 standard)
    if (width == 0 || width == -1)
        str->Type() = std::make_unique<CArrayType>(
            std::make_unique<CArithmType>(TypeTag::int8));
    else if (width == 1)
        str->Type() = std::make_unique<CArrayType>(
            std::make_unique<CArithmType>(TypeTag::uint8));
    else if (width == 2)
        str->Type() = std::make_unique<CArrayType>(
            std::make_unique<CArithmType>(TypeTag::uint16));
    else if (width == 4)
        str->Type() = std::make_unique<CArrayType>(
            std::make_unique<CArithmType>(sign ? TypeTag::int32 : TypeTag::uint32));
}


void TypeBuilder::VisitUnaryExpr(UnaryExpr* expr)
{
    if (expr->Type()) return;

    expr->Content()->Accept(this);
    if (expr->Op() == Tag::asterisk)
    {
        auto ptr = expr->Content()->Type();
        if (ptr->Is<CPtrType>())
            expr->Type() = ptr->As<CPtrType>()->Point2()->Clone();
        else if (ptr->Is<CArrayType>())
            expr->Type() = ptr->As<CArrayType>()->ArrayOf()->Clone();
    }
    else if (expr->Op() == Tag::_and)
    {
        auto ty = expr->Content()->Type()->Clone();
        expr->Type() = std::make_unique<CPtrType>(std::move(ty));
    }
    else
        expr->Type() = expr->Content()->Type();
}
