#include "SysVConv.h"
#include "IR/IROperand.h"
#include "IR/IRType.h"


void SysVConv::CheckParamClass(const IRType* t)
{
    if (t->IsInt())
        wordclass_.push_back(ParamClass::integer);
    else if (t->IsFloat() && t->Size() < 16)
        wordclass_.push_back(ParamClass::sse);
    else if (t->IsFloat())
    {
        wordclass_.push_back(ParamClass::x87);
        wordclass_.push_back(ParamClass::x87up);
    }

    // for sort of things like _float128, _Decimal128...
    // Well, I don't care about them, at least for now

    // We're done if t is not aggregate type
    // array is treated as a pointer
    if (t->IsArray())
        wordclass_.push_back(ParamClass::integer);
}

void SysVConv::Emplace(int i, const IRType* p, RegTag t)
{
    if (t == RegTag::none) return;
    argvs_[i] = std::make_unique<x64Reg>(t, p->Size());
}

RegTag SysVConv::GetIntReg()
{
    switch (intcnt_++)
    {
    case 1: return RegTag::rdi;
    case 2: return RegTag::rsi;
    case 3: return RegTag::rdx;
    case 4: return RegTag::rcx;
    case 5: return RegTag::r8;
    case 6: return RegTag::r9;
    default: return RegTag::none;
    }
}

RegTag SysVConv::GetVecReg()
{
    switch (veccnt_++)
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


SysVConv::SysVConv(const FuncType* f)
{
#define ALIGN_BY_8(x) (((x) + 7) & ~7)

    auto size = f->ParamType().size();
    for (int i = 0; i < size; ++i)
        CheckParamClass(f->ParamType()[i]);

    // index for wordclass_
    int windex = 0;
    for (int i = 0; i < size; ++i)
    {
        switch (wordclass_[windex])
        {
        case ParamClass::integer:
            Emplace(i, f->ParamType()[i], GetIntReg());
            windex++;
            break;
        case ParamClass::sse:
            Emplace(i, f->ParamType()[i], GetVecReg());
            windex++;
            break;

        // no need to implement this case - we
        // don't even have __m256 or __m512.
        // case ParamClass::sseup: break;

        // If the class is X87, X87UP or COMPLEX_X87,
        // it is passed in memory.
        case ParamClass::x87:
            windex += 2; // since x87up always follows behind x87
            continue;
        case ParamClass::memory:
            stacksize_ += f->ParamType()[i]->Size();
            stacksize_ = ALIGN_BY_8(stacksize_);
            windex++;
            continue;
        }
    }

#undef ALIGN_BY_8
}


const x64* SysVConv::PlaceOfArgv(int index) const
{
    if (argvs_.find(index) != argvs_.end())
        return argvs_.at(index).get();
    return nullptr;
}
