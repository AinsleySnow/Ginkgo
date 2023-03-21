#include "ast/Declaration.h"
#include "visitast/Visitor.h"


Declaration* Declaration::InnerMost()
{
    if (child_)
        return child_->InnerMost();
    return this;
}


void DeclSpec::Accept(Visitor* v)
{
    v->VisitDeclSpec(this);
}

void DeclSpec::AddTypeSpec(std::unique_ptr<::TypeSpec> ts)
{
    speclist_.push_back(std::move(ts));
}

TypeTag DeclSpec::TypeSpec()
{
    for (auto& tyspec : speclist_)
        SetRawSpec(tyspec->Spec());

    switch (rawspec_)
    {
    // int and its equivalence
    case int(Tag::_int): case int(Tag::_signed):
    case int(Tag::_int) | int(Tag::_signed):
        return TypeTag::int32;
    case int(Tag::_int) | int(Tag::_unsigned):
    case int(Tag::_unsigned):
        return TypeTag::uint32;

    // short and its equivalence
    case int(Tag::_short): case int(Tag::_short) | int(Tag::_int):
    case int(Tag::_signed) | int(Tag::_short):
    case int(Tag::_signed) | int(Tag::_short) | int(Tag::_int):
        return TypeTag::int16;
    case int(Tag::_short) | int(Tag::_unsigned):
    case int(Tag::_short) | int(Tag::_unsigned) | int(Tag::_int):
        return TypeTag::uint16;

    // long and its equivalence
    case int(Tag::_longlong): case int(Tag::_long):
    case int(Tag::_long) | int(Tag::_signed):
    case int(Tag::_long) | int(Tag::_int):
    case int(Tag::_signed) | int(Tag::_long) | int(Tag::_int):
    case int(Tag::_longlong) | int(Tag::_signed):
    case int(Tag::_longlong) | int(Tag::_int):
    case int(Tag::_signed) | int(Tag::_longlong) | int(Tag::_int):
        return TypeTag::int64;
    case int(Tag::_unsigned) | int(Tag::_longlong):
    case int(Tag::_unsigned) | int(Tag::_long):
    case int(Tag::_long) | int(Tag::_int) | int(Tag::_unsigned): 
    case int(Tag::_unsigned) | int(Tag::_longlong) | int(Tag::_int):
        return TypeTag::uint64;

    // char and its equivalence
    case int(Tag::_bool):
        return TypeTag::int8;

    case int(Tag::_char):
    case int(Tag::_char) | int(Tag::_signed):
        return TypeTag::int8;
    case int(Tag::_char) | int(Tag::_unsigned):
        return TypeTag::int8;

    // double
    case int(Tag::_double): case int(Tag::_double) | int(Tag::_long):
        return TypeTag::flt64;

    // float
    case int(Tag::_float):
        return TypeTag::flt32;

    // void
    case int(Tag::_void):
        return TypeTag::_void;

    // others
    default: return TypeTag::customed;
    }
}

QualType DeclSpec::Qual()
{
    QualType qual{};
    for (auto tag : quallist_)
        qual.SetToken(tag);
    return qual;
}

StorageType DeclSpec::Storage()
{
    StorageType storage{};
    for (auto tag : storagelist_)
        storage.SetToken(tag);
    return storage;
}

FuncSpec DeclSpec::Func()
{
    FuncSpec func{};
    for (auto tag : funcspeclist_)
        func.SetSpec(tag);
    return func;
}

bool DeclSpec::SetRawSpec(Tag t)
{
    if (t == Tag::_long)
    {
        if (!(rawspec_ & unsigned(Tag::_long)))
            rawspec_ |= unsigned(Tag::_long);
        else
        {
            if (!(rawspec_ & (unsigned(Tag::_long) << 1)))
                rawspec_ |= unsigned(Tag::_long) << 1;
            else return false;
        }
        return true;
    }

    if (!(rawspec_ & unsigned(t)))
        rawspec_ |= static_cast<unsigned>(t);
    else return false;

    return true;
}


void ParamList::Accept(Visitor* v)
{
    v->VisitParamList(this);
}

void ParamList::Append(std::unique_ptr<Declaration> decl)
{
    paramlist_.push_back(std::move(decl));
}

void ParamList::AppendType(const CType* ty)
{
    paramtype_.push_back(ty);
}


void DeclList::Accept(Visitor* v)
{
    v->VisitDeclList(this);
}

void DeclList::Append(std::unique_ptr<InitDecl> decl)
{
    decllist_.push_back(std::move(decl));
}

void ObjDef::Accept(Visitor* v)
{
    v->VisitObjDef(this);
}

void PtrDef::Accept(Visitor* v)
{
    v->VisitPtrDef(this);
}

void ArrayDef::Accept(Visitor* v)
{
    v->VisitArrayDef(this);
}

void FuncDef::Accept(Visitor* v)
{
    v->VisitFuncDef(this);
}
