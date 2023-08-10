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
    if (t->Is<ArrayType>())
        wordclass_.push_back(ParamClass::integer);
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


SysVConv::SysVConv(const FuncType* f) : functype_(f)
{
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
        case ParamClass::x87:
        case ParamClass::memory:
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
