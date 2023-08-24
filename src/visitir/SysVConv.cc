#include "visitir/SysVConv.h"
#include "IR/IROperand.h"
#include "IR/IRType.h"


void SysVConv::AlignStackBy(size_t add, size_t align)
{
    if ((stacksize_ + add) % align == 0)
        return;
    stacksize_ += align - (stacksize_ + add) % align;
}

void SysVConv::CheckParamClass(const IRType* t)
{
    if (t->Is<IntType>())
        wordclass_.push_back(ParamClass::integer);
    else if (t->Is<FloatType>() && t->Size() < 16)
        wordclass_.push_back(ParamClass::sse);
    else if (t->Is<FloatType>())
    {
        wordclass_.push_back(ParamClass::x87);
        wordclass_.push_back(ParamClass::x87up);
    }

    // for sort of things like _float128, _Decimal128...
    // Well, I don't care about them, at least for now

    // We're done if t is not aggregate type
    // array is treated as a pointer
    else if (t->Is<ArrayType>())
        wordclass_.push_back(ParamClass::integer);

    // if a struct or union is greater than 64 bytes,
    // then it is passed on the stack
    else if (t->Is<HeterType>() && t->Size() > 64)
        wordclass_.push_back(ParamClass::memory);

    else if (t->Is<HeterType>())
    {
        for (size_t i = 0; i < t->Size(); i += 8)
        {
            if (HasFloat(t, i, i + 8, 0))
                wordclass_.push_back(ParamClass::sse);
            else
                wordclass_.push_back(ParamClass::integer);
        }
    }
}

void SysVConv::Emplace(int i, const IRType* p, RegTag t)
{
    if (t == RegTag::none)
    {
        AlignStackBy(0, p->Align());
        memoffset_[i] = stacksize_;
        stacksize_ += 8;
        return;
    }
    argvs_[i] = std::make_unique<x64Reg>(t, p->Size());
}

void SysVConv::Emplace(x64Heter* h, RegTag tag)
{
    if (tag != RegTag::none)
        h->Map2Reg(tag);
    else
    {
        AlignStackBy(0, 8);
        h->Map2Mem(stacksize_);
        stacksize_ += 8;
    }
}

RegTag SysVConv::GetIntReg()
{
    if (intcnt_ == 6)
        return RegTag::none;
    return Index2IntTag(intcnt_++);
}

RegTag SysVConv::GetVecReg()
{
    if (veccnt_ == 8)
        return RegTag::none;
    return Index2VecTag(veccnt_++);
}


// from chibicc, codegen.c:
// Structs or unions equal or smaller than 16 bytes are passed
// using up to two registers.
//
// If the first 8 bytes contains only floating-point type members,
// they are passed in an XMM register. Otherwise, they are passed
// in a general-purpose register.
//
// If a struct/union is larger than 8 bytes, the same rule is
// applied to the the next 8 byte chunk.
// The HasFloat method here is adapted from has_flonum in chibicc.
bool SysVConv::HasFloat(const IRType* ty, size_t lo, size_t hi, size_t offset)
{
    if (ty->Is<HeterType>())
    {
        auto h = ty->As<HeterType>();
        for (auto m : *h)
            if (!HasFloat(m.first, lo, hi, m.second))
                return false;
        return true;
    }
    if (ty->Is<ArrayType>())
    {
        auto array = ty->As<ArrayType>();
        for (int i = 0; i < array->Count(); ++i)
            if (!HasFloat(array->ArrayOf(), lo, hi,
                offset + array->ArrayOf()->Size() * i))
                return false;
        return true;
    }
    return offset < lo || hi <= offset || ty->Is<FloatType>();
}


SysVConv::SysVConv(const FuncType* f) : functype_(f)
{
    // If the function returns a big structure/union,
    // then rdi is used for storing the address of the return value.
    // On return %rax will contain the address that has been
    // passed in by the caller in %rdi.
    if (f->ReturnType()->Is<HeterType>() && f->ReturnType()->Size() > 16)
        intcnt_ = 1;
    for (int i = 0; i < f->ParamType().size(); ++i)
        CheckParamClass(f->ParamType()[i]);
}

void SysVConv::MapArgv()
{
    auto size = functype_->ParamType().size();

    // index for wordclass_
    int windex = 0;
    for (int i = 0; i < size; ++i)
    {
        if (functype_->ParamType()[i]->Is<HeterType>() &&
            functype_->ParamType()[i]->Size() <= 64)
        {
            auto sz = functype_->ParamType()[i]->Size();
            auto heter = std::make_unique<x64Heter>(sz);
            for (int index = 0; index < sz; index += 8)
            {
                if (wordclass_[windex] == ParamClass::integer)
                    Emplace(heter.get(), GetIntReg());
                else // if the eight bytes is classified into sse class
                    Emplace(heter.get(), GetVecReg());
                windex += 1;
            }
            argvs_[i] = std::move(heter);
            continue;
        }

        switch (wordclass_[windex])
        {
        case ParamClass::integer:
            Emplace(i, functype_->ParamType()[i], GetIntReg());
            windex++;
            break;
        case ParamClass::sse:
            Emplace(i, functype_->ParamType()[i], GetVecReg());
            windex++;
            break;

        // no need to implement this case - we
        // don't even have __m256 or __m512.
        // case ParamClass::sseup: break;

        // If the class is X87, X87UP or COMPLEX_X87,
        // it is passed in memory.
        // If the class is MEMORY, pass the argument on the stack
        // at an address respecting the arguments alignment (which
        // might be more than its natural alignement).
        // As when passing parameters, the stack is always aligned by 8,
        // which is fit for all arithmetic types, only when the
        // type is a struct or union may the 8-byte alignment become unfit.
        // Hence we only change the align size when the type is a
        // heterogeneous type with alignment greater than 8.
        // A worth to note annoying case is that, if the type is over
        // aligned, e.g., being 512-byte aligned, the simple approach
        // here cannot guarantee the desired alignment. I would like to
        // ignore the case for now.
        case ParamClass::x87:
        case ParamClass::memory:
            if (auto ty = functype_->ParamType()[i];
                ty->Is<HeterType>() && ty->Align() > 8)
                AlignStackBy(0, ty->Align());
            else
                AlignStackBy(0, 8);
            memoffset_[i] = static_cast<long>(stacksize_);
            stacksize_ += functype_->ParamType()[i]->Size();

            if (wordclass_[windex] == ParamClass::x87)
                windex += 2; // since x87up always follows behind x87
            else
                windex++;
            break;
        }
    }

    auto oldsz = stacksize_;
    AlignStackBy(8, 16);
    padding_ = stacksize_ - oldsz;
}

const x64* SysVConv::PlaceOfArgv(int index) const
{
    if (argvs_.find(index) != argvs_.end())
        return argvs_.at(index).get();
    return nullptr;
}

long SysVConv::OffsetOfArgv(int index) const
{
    if (memoffset_.find(index) != memoffset_.end())
        return memoffset_.at(index);
    return -1;
}

std::unique_ptr<x64>&& SysVConv::ExtractArgv(int i)
{
    return std::move(argvs_.at(i));
}


RegTag SysVConv::Index2IntTag(int i)
{
    switch (i)
    {
    case 0: return RegTag::rdi;
    case 1: return RegTag::rsi;
    case 2: return RegTag::rdx;
    case 3: return RegTag::rcx;
    case 4: return RegTag::r8;
    case 5: return RegTag::r9;
    default: return RegTag::none;
    }
}

RegTag SysVConv::Index2VecTag(int i)
{
    switch (i)
    {
    case 0: return RegTag::xmm0;
    case 1: return RegTag::xmm1;
    case 2: return RegTag::xmm2;
    case 3: return RegTag::xmm3;
    case 4: return RegTag::xmm4;
    case 5: return RegTag::xmm5;
    case 6: return RegTag::xmm6;
    case 7: return RegTag::xmm7;
    default: return RegTag::none;
    }
}

std::pair<int, int> SysVConv::CountRegs() const
{
    int gp = 0, vec = 0;
    int windex = 0;
    for (int i = 0; i < functype_->ParamType().size(); ++i)
    {
        switch (wordclass_[windex])
        {
        case ParamClass::integer:
            gp++; windex++; break;
        case ParamClass::sse:
            vec++; windex++; break;
        case ParamClass::x87:
            windex += 2; break;
        case ParamClass::memory:
            windex++; break;
        }
    }

    return std::make_pair(gp, vec);
}
