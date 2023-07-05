#include "visitir/CodeGen.h"
#include "visitir/SysVConv.h"
#include "visitir/x64.h"
#include "IR/Value.h"
#include <memory>
#include <string>


static std::string BB2Label(const BasicBlock* bb)
{
    return ".L" + bb->Name();
}

static std::string Cond2Str(Condition cond, bool issigned)
{
    std::string str = "";
    switch (cond)
    {
    case Condition::eq: str = "e"; break;
    case Condition::ne: str = "ne"; break;
    case Condition::gt: str = issigned ? "g" : "a"; break;
    case Condition::le: str = issigned ? "le" : "be"; break;
    case Condition::ge: str = issigned ? "ge" : "ae"; break;
    case Condition::lt: str = issigned ? "l" : "b"; break;
    }

    return str;
}

static RegTag X64Phys2RegTag(x64Phys phys)
{
    return static_cast<RegTag>(static_cast<int>(phys) + 2);
}

static x64Phys RegTag2X64Phys(RegTag tag)
{
    return static_cast<x64Phys>(static_cast<int>(tag) - 2);
}


std::string CodeGen::GetLCLabel() const
{
    return ".LC" + std::to_string(labelindex_++);
}

std::string CodeGen::GetTempLabel() const
{
    return ".T" + std::to_string(labelindex_++);
}


const x64* CodeGen::MapPossibleFloat(const IROperand* op)
{
    auto fp = op->As<FloatConst>();
    if (!fp)
        return alloc_.GetIROpMap(op);

    if (auto res = tempmap_.find(op); res != tempmap_.end())
        return res->second.get();

    auto val = fp->Val();
    std::string label = "";
    if (auto l = fpconst_.find(val); l != fpconst_.end())
        label = l->second;
    else
    {
        label = GetLCLabel();
        auto imm = alloc_.GetIROpMap(op)->As<x64Imm>();
        asmfile_.EmitLabel(label);
        asmfile_.EmitPseudoInstr(".long", { std::to_string(imm->GetRepr()) });
        fpconst_[val] = label;
    }

    tempmap_[op] = std::make_unique<x64Mem>(op->Type()->Size(), label);
    return tempmap_[op].get();
}

const x64* CodeGen::MapPossiblePointer(const IROperand* op)
{
    if (!op->Type()->Is<PtrType>())
        return alloc_.GetIROpMap(op);

    auto mappedop = alloc_.GetIROpMap(op);
    auto mayload = RegTag::none;

    if (mappedop->Is<x64Mem>() && mappedop->As<x64Mem>()->LoadTwice())
    {
        auto regs = alloc_.NotUsedIntReg();
        if (regs.empty())
            mayload = RegTag::rax;
        else
            mayload = X64Phys2RegTag(*regs.begin());
        asmfile_.EmitMov(mappedop, mayload);
        tempmap_[op] = std::make_unique<x64Mem>(8, 0, mayload, RegTag::none, 0);
    }
    else if (mappedop->Is<x64Reg>())
    {
        // Address in a register. Convert it to a x64Mem.
        auto reg = mappedop->As<x64Reg>();
        tempmap_[op] = std::make_unique<x64Mem>(8, 0, *reg, RegTag::none, 0);
    }
    else
        return mappedop;

    return tempmap_.at(op).get();
}


void CodeGen::AlignRspBy(size_t add, size_t align)
{
    if ((stacksize_ + add) % align == 0)
        return;
    auto offset = align - (stacksize_ + add) % align;
    AdjustRsp(-offset);
}

void CodeGen::AdjustRsp(long offset)
{
    x64Reg rsp{ RegTag::rsp };
    if (offset < 0)
        asmfile_.EmitBinary("sub", -offset, &rsp);
    else if (offset > 0)
        asmfile_.EmitBinary("add", offset, &rsp);
    stacksize_ += (-offset);
}

void CodeGen::DeallocFrame()
{
    AdjustRsp(stacksize_);
}


void CodeGen::PassParam(
    const SysVConv& conv, const std::vector<const IROperand*>& param)
{
    for (int i = param.size() - 1; i >= 0; --i)
    {
        auto loc = conv.PlaceOfArgv(i);
        if (!loc)
            PushEmitHelper(MapPossibleFloat(param[i]));
        else
        {
            auto reg = loc->As<x64Reg>();
            asmfile_.EmitMov(MapPossibleFloat(param[i]), loc);            
            continue;
        }
    }
}

void CodeGen::SaveCalleeSaved()
{
    auto regs = alloc_.UsedCalleeSaved();
    for (auto reg : regs)
        PushEmitHelper(X64Phys2RegTag(reg), 8);
}

void CodeGen::RestoreCalleeSaved()
{
    auto regs = alloc_.UsedCalleeSaved();
    for (auto i = regs.rbegin(); i != regs.rend(); ++i)
        PopEmitHelper(X64Phys2RegTag(*i), 8);
}

void CodeGen::SaveCallerSaved()
{
    auto regs = alloc_.UsedCallerSaved();
    for (auto reg: regs)
        PushEmitHelper(X64Phys2RegTag(reg), 8);
}

void CodeGen::RestoreCallerSaved()
{
    auto regs = alloc_.UsedCallerSaved();
    for (auto i = regs.rbegin(); i != regs.rend(); ++i)
        PopEmitHelper(X64Phys2RegTag(*i), 8);
}


void CodeGen::BinaryGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = alloc_.GetIROpMap(bi->Lhs());
    auto rhs = alloc_.GetIROpMap(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());

    asmfile_.EmitBinary(name, rhs, lhs);
    if (lhs->operator!=(*ans))
        asmfile_.EmitMov(lhs, ans);
}

void CodeGen::VarithmGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = MapPossibleFloat(bi->Lhs());
    auto rhs = MapPossibleFloat(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());
    asmfile_.EmitVarithm(name, lhs, rhs, ans);
}


void CodeGen::LeaqEmitHelper(const x64* addr, const x64* dest)
{
    auto mem = addr->As<x64Mem>();
    if (dest->Is<x64Reg>())
    {
        asmfile_.EmitLeaq(mem, dest);
        return;
    }

    auto notused = alloc_.NotUsedIntReg();
    if (notused.empty())
    {
        asmfile_.EmitLeaq(mem, RegTag::rax);
        asmfile_.EmitMov(RegTag::rax, dest);
    }
    else
    {
        RegTag reg = X64Phys2RegTag(*notused.begin());
        asmfile_.EmitLeaq(addr, reg);
        asmfile_.EmitMov(reg, dest);
    }
}

void CodeGen::MovEmitHelper(const x64* from, const x64* to)
{
    if (from->Is<x64Reg>() || from->Is<x64Imm>() || to->Is<x64Reg>())
    {
        asmfile_.EmitMov(from, to);
        return;
    }

    auto notused = alloc_.NotUsedIntReg();
    if (notused.empty())
    {
        asmfile_.EmitMov(from, RegTag::rax);
        asmfile_.EmitMov(RegTag::rax, to);
    }
    else
    {
        auto mayspill = *notused.begin();
        auto tag = X64Phys2RegTag(mayspill);
        asmfile_.EmitMov(from, tag);
        asmfile_.EmitMov(tag, to);
    }
}


// the destination of movz or movs must be a register
#define MOVZ_MOVS_HELPER(name)                                  \
if (to->Is<x64Reg>())                                           \
{                                                               \
    asmfile_.name(from, to);                                    \
    return;                                                     \
}                                                               \
                                                                \
auto notused = alloc_.NotUsedIntReg();                          \
if (notused.empty())                                            \
{                                                               \
    x64Reg rax{ RegTag::rax, to->Size() };                      \
    asmfile_.name(from, &rax);                                  \
    asmfile_.EmitMov(&rax, to);                                 \
}                                                               \
else                                                            \
{                                                               \
    x64Reg reg{ X64Phys2RegTag(*notused.begin()), to->Size() }; \
    asmfile_.name(from, &reg);                                  \
    asmfile_.EmitMov(&reg, to);                                 \
}

void CodeGen::MovzEmitHelper(const x64* from, const x64* to) { MOVZ_MOVS_HELPER(EmitMovz); }
void CodeGen::MovsEmitHelper(const x64* from, const x64* to) { MOVZ_MOVS_HELPER(EmitMovs); }

#undef MOVZ_MOVS_HELPER

#define MOVZ_MOVS_HELPER(name)                              \
if (op->Is<x64Reg>() && from == to) /* In the same reg? */  \
    return;                         /* Emit nothing */      \
if (op->Is<x64Reg>())                                       \
{                                                           \
    asmfile_.name(from, to, op);                            \
    return;                                                 \
}                                                           \
                                                            \
auto notused = alloc_.NotUsedIntReg();                      \
if (notused.empty())                                        \
{                                                           \
    x64Reg rax{ RegTag::rax, from };                        \
    asmfile_.EmitMov(op, &rax);                             \
    asmfile_.name(from, to, &rax);                          \
    asmfile_.EmitMov(&rax, op);                             \
}                                                           \
else                                                        \
{                                                           \
    x64Reg reg{ X64Phys2RegTag(*notused.begin()), from };   \
    asmfile_.EmitMov(op, &reg);                             \
    asmfile_.name(from, to, &reg);                          \
    asmfile_.EmitMov(&reg, op);                             \
}

void CodeGen::MovzEmitHelper(size_t from, size_t to, const x64* op) { MOVZ_MOVS_HELPER(EmitMovz); }
void CodeGen::MovsEmitHelper(size_t from, size_t to, const x64* op) { MOVZ_MOVS_HELPER(EmitMovs); }

#undef MOVZ_MOVS_HELPER


void CodeGen::VecMovEmitHelper(const x64* src, const x64* dest)
{
    if (src->Is<x64Reg>() && dest->Is<x64Reg>())
        asmfile_.EmitVmovap(src->As<x64Reg>(), dest->As<x64Reg>());
    else if (!src->Is<x64Reg>() && dest->Is<x64Reg>() ||
              src->Is<x64Reg>() && !dest->Is<x64Reg>())
        asmfile_.EmitVmov(src, dest);
    else
    {
        auto notused = alloc_.NotUsedVecReg();
        if (notused.empty())
        {
            PushXmmReg(RegTag::xmm0);
            asmfile_.EmitVmov(src, RegTag::xmm0);
            asmfile_.EmitVmov(RegTag::xmm0, dest);
            PopXmmReg(RegTag::xmm0);
        }
        else
        {
            auto mayspill = *notused.begin();
            auto tag = X64Phys2RegTag(mayspill);
            asmfile_.EmitVmov(src, tag);
            asmfile_.EmitVmov(tag, dest);
        }
    }
}

void CodeGen::VecMovEmitHelper(const x64* src, RegTag dest)
{
    if (src->Is<x64Reg>())
        asmfile_.EmitVmovap(src->As<x64Reg>(), dest);
    else
        asmfile_.EmitVmov(src, dest);
}

void CodeGen::VecMovEmitHelper(RegTag src, const x64* dest)
{
    if (dest->Is<x64Reg>())
        asmfile_.EmitVmovap(src, dest->As<x64Reg>());
    else
        asmfile_.EmitVmov(src, dest);
}


void CodeGen::VcvtEmitHelper(const x64* op1, const x64* op2)
{
    if (op2->Is<x64Reg>())
    {
        asmfile_.EmitVcvt(op1, op2);
        return;
    }

    auto notused = alloc_.NotUsedVecReg();
    if (notused.empty())
    {
        PushXmmReg(RegTag::xmm0);
        asmfile_.EmitVcvt(op1, RegTag::xmm0);
        asmfile_.EmitVcvt(RegTag::xmm0, op2);
        PopXmmReg(RegTag::xmm0);
    }
    else
    {
        auto mayspill = *notused.begin();
        auto tag = X64Phys2RegTag(mayspill);
        asmfile_.EmitVcvt(op1, tag);
        asmfile_.EmitVcvt(tag, op2);
    }
}

void CodeGen::VcvttEmitHelper(const x64* op1, const x64* op2)
{
    if (op2->Is<x64Reg>())
    {
        asmfile_.EmitVcvtt(op1, op2);
        return;
    }

    auto notused = alloc_.NotUsedVecReg();
    if (notused.empty())
    {
        PushXmmReg(RegTag::xmm0);
        asmfile_.EmitVcvtt(op1, RegTag::xmm0);
        asmfile_.EmitVcvtt(RegTag::xmm0, op2);
        PopXmmReg(RegTag::xmm0);
    }
    else
    {
        auto mayspill = *notused.begin();
        auto tag = X64Phys2RegTag(mayspill);
        asmfile_.EmitVcvtt(op1, tag);
        asmfile_.EmitVcvtt(tag, op2);
    }
}


void CodeGen::PushEmitHelper(const x64* reg)
{
    auto original = 0;

    if (reg->Size() != 8)
    {
        MovzEmitHelper(reg->Size(), 8, reg);
        original = reg->Size();
        const_cast<x64*>(reg)->Size() = 8;
    }
    stacksize_ += reg->Size();
    asmfile_.EmitPush(reg);

    if (original)
        const_cast<x64*>(reg)->Size() = original;
}

void CodeGen::PushEmitHelper(RegTag tag, size_t size)
{
    if (size != 8)
    {
        x64Reg reg{ tag };
        asmfile_.EmitMovz(size, 8, &reg);
        size = 8;
    }
    asmfile_.EmitPush(tag, size);
    stacksize_ += size;
}

void CodeGen::PopEmitHelper(const x64* reg)
{
    size_t original = 0;
    if (reg->Size() != 8)
    {
        original = reg->Size();
        const_cast<x64*>(reg)->Size() = 8;
    }
    asmfile_.EmitPop(reg);
    stacksize_ -= reg->Size();
    if (original)
        const_cast<x64*>(reg)->Size() = original;
}

void CodeGen::PopEmitHelper(RegTag tag, size_t size)
{
    if (size != 8)
        size = 8;
    asmfile_.EmitPop(tag, size);
    stacksize_ -= size;
}

void CodeGen::PushXmmReg(RegTag tag)
{
    AdjustRsp(-16);
    x64Reg rsp{ RegTag::rsp, 16 };
    asmfile_.EmitVmovap(tag, &rsp);
}

void CodeGen::PopXmmReg(RegTag tag)
{
    x64Reg rsp{ RegTag::rsp, 16 };
    asmfile_.EmitVmovap(&rsp, tag);
    AdjustRsp(16);
}


void CodeGen::VisitModule(Module* mod)
{
    asmfile_.EmitPseudoInstr(".file", { "\"" + mod->Name() + "\"" });
    asmfile_.EmitBlankLine();
    for (auto val : *mod)
        val->Accept(this);
}

void CodeGen::VisitGlobalVar(GlobalVar* var)
{
    // stripping the leading '@'
    auto name = var->Name().substr(1);
    auto size = var->Type()->Size();

    var->GetTree()->Accept(this);
    auto repr = var->GetTree()->repr_;
    if (repr[0] == '"')
    {
        asmfile_.EmitPseudoInstr(".section .rodata");
        asmfile_.EmitLabel(name);
        auto width = dynamic_cast<OpNode*>(var->GetTree())->op_->
            As<StrConst>()->Type()->As<ArrayType>()->ArrayOf()->Size();
        std::string pseudo = ".string";
        if (width != 1)
            pseudo += std::to_string(width * 8);

        asmfile_.EmitPseudoInstr(std::move(pseudo), { std::move(repr) });
    }
    else
    {
        auto strsz = std::to_string(size);
        asmfile_.EmitPseudoInstr(".data");
        asmfile_.EmitPseudoInstr(".globl", { name });
        asmfile_.EmitPseudoInstr(".align", { strsz });
        asmfile_.EmitPseudoInstr(".type", { name, "@object" });
        asmfile_.EmitPseudoInstr(".size", { name, strsz });
        asmfile_.EmitLabel(name);

        if (!var->GetTree())
            asmfile_.EmitPseudoInstr(".zero", { strsz });
        else
        {
            std::string pseudo = "";
            switch (size)
            {
            case 1: pseudo = ".byte"; break;
            case 2: pseudo = ".value"; break;
            case 4: pseudo = ".long"; break;
            case 8: pseudo = ".quad"; break;
            }
            asmfile_.EmitPseudoInstr(pseudo, { var->GetTree()->repr_ });
        }
    }
    asmfile_.EmitBlankLine();
}

void CodeGen::VisitFunction(Function* func)
{
    if (func->Empty())
        return;

    alloc_.EnterFunction(func);
    auto name = func->Name().substr(1);

    asmfile_.EmitPseudoInstr(".text");
    asmfile_.EmitPseudoInstr(".globl", { name });
    asmfile_.EmitPseudoInstr(".type", { name, "@function" });
    asmfile_.EmitLabel(func->Name().substr(1));

    asmfile_.EmitPush(RegTag::rbp, 8);
    x64Reg rsp{ RegTag::rsp };
    x64Reg rbp{ RegTag::rbp };
    asmfile_.EmitMov(&rsp, &rbp);
    AdjustRsp(-alloc_.RspOffset());

    if (func->Variadic())
    {
        AlignRspBy(0, 8);
        auto baseline = stacksize_;
        AdjustRsp(-176);
        asmfile_.EmitMov(RegTag::rdi, baseline);
        asmfile_.EmitMov(RegTag::rsi, baseline + 8);
        asmfile_.EmitMov(RegTag::rdx, baseline + 16);
        asmfile_.EmitMov(RegTag::rcx, baseline + 24);
        asmfile_.EmitMov(RegTag::r8, baseline + 32);
        asmfile_.EmitMov(RegTag::r9, baseline + 40);

        auto rax = x64Reg(RegTag::rax, 1);
        auto label = GetTempLabel();
        asmfile_.EmitTest(&rax, &rax);
        asmfile_.EmitJmp("e", label);

        asmfile_.EmitVmov(RegTag::xmm0, baseline + 48);
        asmfile_.EmitVmov(RegTag::xmm1, baseline + 64);
        asmfile_.EmitVmov(RegTag::xmm2, baseline + 80);
        asmfile_.EmitVmov(RegTag::xmm3, baseline + 96);
        asmfile_.EmitVmov(RegTag::xmm4, baseline + 112);
        asmfile_.EmitVmov(RegTag::xmm5, baseline + 128);
        asmfile_.EmitVmov(RegTag::xmm6, baseline + 144);
        asmfile_.EmitVmov(RegTag::xmm7, baseline + 160);

        asmfile_.EmitLabel(label);
    }

    SaveCalleeSaved();
    asmfile_.Write2Mem();

    for (auto bb : *func)
        VisitBasicBlock(bb);

    asmfile_.Dump2File();
    asmfile_.EmitBlankLine();
}

void CodeGen::VisitBasicBlock(BasicBlock* bb)
{
    asmfile_.EnterBlock(bb);
    asmfile_.EmitLabel(BB2Label(bb));
    for (auto inst : *bb)
        inst->Accept(this);
}


void CodeGen::VisitNode(OpNode* op)
{
    if (op->op_->Is<IntConst>())
        op->repr_ = std::to_string(op->op_->As<IntConst>()->Val());
    else if (op->op_->Is<FloatConst>())
        op->repr_ = std::to_string(op->op_->As<FloatConst>()->Val());
    else if (op->op_->Is<StrConst>())
        op->repr_ = op->op_->As<StrConst>()->ToString();
    else // strip the leading '@'
        op->repr_ = op->op_->As<Register>()->Name().substr(1);
}

void CodeGen::VisitNode(BinaryNode* bin)
{
    bin->left_->Accept(this);
    bin->right_->Accept(this);

    if (bin->id_ == Instr::InstrId::add)
        bin->repr_ = std::move(bin->left_->repr_) + '+' + std::move(bin->right_->repr_);
    else if (bin->id_ == Instr::InstrId::sub)
        bin->repr_ = std::move(bin->left_->repr_) + '-' + std::move(bin->right_->repr_);
}

void CodeGen::VisitNode(UnaryNode* un)
{
    un->op_->Accept(this);
    if (un->id_ == Instr::InstrId::geteleptr)
        un->repr_ = std::move(un->op_->repr_);
}


void CodeGen::VisitRetInstr(RetInstr* inst)
{
    if (!inst->ReturnValue())
        goto ret;
    else if (inst->ReturnValue()->Type()->Is<IntType>())
    {
        auto mapped = alloc_.GetIROpMap(inst->ReturnValue());
        if (mapped->Is<x64Reg>() && *mapped->As<x64Reg>() == RegTag::rax)
            goto ret;
        asmfile_.EmitMov(mapped, RegTag::rax);
    }
    else if (inst->ReturnValue()->Type()->Is<FloatType>())
    {
        auto mapped = MapPossibleFloat(inst->ReturnValue());
        if (mapped->Is<x64Reg>() && *mapped->As<x64Reg>() == RegTag::xmm0)
            goto ret;
        VecMovEmitHelper(MapPossibleFloat(inst->ReturnValue()), RegTag::xmm0);
    }

ret:
    // what if there's multiple ret instruction in a function?
    auto oldstacksize = stacksize_;
    RestoreCalleeSaved();
    DeallocFrame();
    stacksize_ = oldstacksize;

    asmfile_.EmitLeave();
    asmfile_.EmitRet();
}


void CodeGen::VisitBrInstr(BrInstr* inst)
{
    if (inst->Cond())
    {
        auto cond = MapPossibleFloat(inst->Cond());
        asmfile_.EmitCmp(cond, (unsigned long)0);
        asmfile_.EmitJmp("ne", BB2Label(inst->GetTrueBlk()));
        asmfile_.EmitJmp("", BB2Label(inst->GetFalseBlk()));
    }
    else asmfile_.EmitJmp("", BB2Label(inst->GetTrueBlk()));
}


void CodeGen::VisitSwitchInstr(SwitchInstr* inst)
{
    // I use a somewhat straight forward way to translate
    // the switch instruction. no jump table is used, though.

    auto ident = alloc_.GetIROpMap(inst->GetIdent());
    for (auto [tag, bb] : inst->GetValueBlkPairs())
    {
        asmfile_.EmitCmp(ident, alloc_.GetIROpMap(tag));
        asmfile_.EmitJmp("z", BB2Label(bb));
    }
    asmfile_.EmitJmp("", BB2Label(inst->GetDefault()));
}


void CodeGen::VisitCallInstr(CallInstr* inst)
{
    // Layout of the stack:
    //         high address
    // +------------------------+
    // |       Local Vars       |
    // +------------------------+
    // |   Caller Saved Regs    |
    // +------------------------+
    // |        Padding         |
    // +------------------------+
    // |  Argvs Passed on Stack |
    // +------------------------+  <- rsp in caller
    // |     Frame of Callee    |
    // +------------------------+
    //         low address

    SaveCallerSaved();
    SysVConv conv{ inst->Proto() };
    conv.MapArgv();

    auto [_, vec] = conv.CountRegs();
    if (inst->Proto()->Variadic())
    {
        if (vec == 0)
            asmfile_.EmitInstr("    xorq %rax, %rax\n");
        else
            asmfile_.EmitInstr("    movb " + std::to_string(vec) + ", %al\n");
    }

    auto extra = (stacksize_ + 8) % 16 ? 8 : 0;
    AdjustRsp(-(conv.Padding() + extra));
    PassParam(conv, inst->ArgvList());
    if (inst->FuncAddr())
        asmfile_.EmitCall(alloc_.GetIROpMap(inst->FuncAddr()));
    else
        asmfile_.EmitCall(inst->FuncName().substr(1) + "@PLT");
    // adjust rsp directly here since conv.StackSize()
    // has already included 'padding'
    AdjustRsp(conv.StackSize() + extra);
    RestoreCallerSaved();

    if (inst->Result() && inst->Result()->Type()->Is<IntType>())
    {
        auto x64reg = alloc_.GetIROpMap(inst->Result());
        if (!x64reg->Is<x64Reg>() || x64reg->As<x64Reg>()->Tag() != RegTag::rax)
            asmfile_.EmitMov(RegTag::rax, x64reg);
    }
    else if (inst->Result() && inst->Result()->Type()->Is<FloatType>())
    {
        auto vecreg = alloc_.GetIROpMap(inst->Result());
        if (!vecreg->Is<x64Reg>() || vecreg->As<x64Reg>()->Tag() != RegTag::xmm0)
            VecMovEmitHelper(RegTag::xmm0, vecreg);
    }
}


void CodeGen::VisitAddInstr(AddInstr* inst)     { BinaryGenHelper("add", inst); }
void CodeGen::VisitFaddInstr(FaddInstr* inst)   { VarithmGenHelper("add", inst); }
void CodeGen::VisitSubInstr(SubInstr* inst)     { BinaryGenHelper("sub", inst); }
void CodeGen::VisitFsubInstr(FsubInstr* inst)   { VarithmGenHelper("sub", inst); }
void CodeGen::VisitMulInstr(MulInstr* inst)     { BinaryGenHelper("imul", inst); }
void CodeGen::VisitFmulInstr(FmulInstr* inst)   { VarithmGenHelper("mul", inst); }
void CodeGen::VisitFdivInstr(FdivInstr* inst)   { VarithmGenHelper("div", inst); }
void CodeGen::VisitShlInstr(ShlInstr* inst)     { BinaryGenHelper("shl", inst); }
void CodeGen::VisitLshrInstr(LshrInstr* inst)   { BinaryGenHelper("shr", inst); }
void CodeGen::VisitAshrInstr(AshrInstr* inst)   { BinaryGenHelper("sar", inst); }
void CodeGen::VisitAndInstr(AndInstr* inst)     { BinaryGenHelper("and", inst); }
void CodeGen::VisitOrInstr(OrInstr* inst)       { BinaryGenHelper("or", inst); }
void CodeGen::VisitXorInstr(XorInstr* inst)     { BinaryGenHelper("xor", inst); }

void CodeGen::VisitDivInstr(DivInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Lhs());
    auto rhs = alloc_.GetIROpMap(inst->Rhs());
    auto ans = alloc_.GetIROpMap(inst->Result());

    // the first op in DivInstr may not be in %*ax.
    // move it to %*ax if it is not.
    if (!lhs->As<x64Reg>()->operator==(RegTag::rax))
        asmfile_.EmitMov(lhs, RegTag::rax);

    asmfile_.EmitCxtx(lhs->Size());

    bool issigned = inst->Lhs()->Type()->As<IntType>()->IsSigned() ||
        inst->Rhs()->Type()->As<IntType>()->IsSigned();
    std::string name = issigned ? "idiv" : "div";
    asmfile_.EmitUnary(name, rhs);

    // if result register is not %*ax, move it to the result register.
    if (!alloc_.GetIROpMap(inst->Result())->As<x64Reg>()->operator==(RegTag::rax))
        asmfile_.EmitMov(RegTag::rax, ans);
}

void CodeGen::VisitModInstr(ModInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Lhs());
    auto rhs = alloc_.GetIROpMap(inst->Rhs());
    auto ans = alloc_.GetIROpMap(inst->Result());

    // the first op in DivInstr may not be in %*ax.
    // move it to %*ax if it is not.
    if (!lhs->As<x64Reg>()->operator==(RegTag::rax))
        asmfile_.EmitMov(lhs, RegTag::rax);

    asmfile_.EmitCxtx(lhs->Size());

    bool issigned = inst->Lhs()->Type()->As<IntType>()->IsSigned() ||
        inst->Rhs()->Type()->As<IntType>()->IsSigned();
    std::string name = issigned ? "idiv" : "div";
    asmfile_.EmitUnary(name, rhs);

    // if result register is not %*dx, move it to the result register.
    if (!alloc_.GetIROpMap(inst->Result())->As<x64Reg>()->operator==(RegTag::rdx))
        asmfile_.EmitMov(RegTag::rdx, ans);
}


void CodeGen::VisitStoreInstr(StoreInstr* inst)
{
    auto dest = inst->Dest();
    auto value = inst->Value();
    auto mappeddest = MapPossiblePointer(dest);

    if (value->Type()->Is<FloatType>())
        VecMovEmitHelper(MapPossibleFloat(value), mappeddest);
    else
        MovEmitHelper(alloc_.GetIROpMap(value), mappeddest);
}

void CodeGen::VisitLoadInstr(LoadInstr* inst)
{
    auto result = inst->Result();
    auto pointer = inst->Pointer();
    auto mappedptr = MapPossiblePointer(pointer);

    if (result->Type()->Is<FloatType>())
        VecMovEmitHelper(mappedptr, MapPossibleFloat(result));
    else
        MovEmitHelper(mappedptr, alloc_.GetIROpMap(result));
}

void CodeGen::VisitGetElePtrInstr(GetElePtrInstr* inst)
{
    auto size = inst->Result()->Type()->Size();
    auto pointer = MapPossiblePointer(inst->Pointer());
    auto index = alloc_.GetIROpMap(inst->OpIndex());
    auto poprbx = false;
    auto indexreg = RegTag::none;
    auto offset = 0;

    if (index->Is<x64Mem>())
    {
        auto notused = alloc_.NotUsedIntReg();
        if (notused.empty())
        {
            indexreg = RegTag::rbx;
            poprbx = true;
            asmfile_.EmitPush(indexreg, 8);
        }
        else
            indexreg = X64Phys2RegTag(*notused.begin());
        asmfile_.EmitMov(index, indexreg);
    }
    else if (index->Is<x64Imm>())
        offset = index->As<x64Imm>()->GetRepr() * size;

    // here, pointer can only be x64Mem*.
    auto castptr = pointer->As<x64Mem>();
    // Is the variable a string?
    if (inst->Pointer()->Name()[1] == '.')
        LeaqEmitHelper(castptr, alloc_.GetIROpMap(inst->Result()));
    else
    {
        x64Mem mem{
            size, castptr->Offset() + offset,
            castptr->Base(), indexreg, size
        };
        MovEmitHelper(&mem, alloc_.GetIROpMap(inst->Result()));
    }

    if (poprbx) asmfile_.EmitPop(RegTag::rbx, 8);
}


void CodeGen::VisitTruncInstr(TruncInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    if (inst->Type()->As<IntType>()->IsSigned())
        MovsEmitHelper(from, to);
    else
        MovzEmitHelper(from, to);
}

void CodeGen::VisitFtruncInstr(FtruncInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}

void CodeGen::VisitZextInstr(ZextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    MovzEmitHelper(from, to);
}

void CodeGen::VisitSextInstr(SextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    MovsEmitHelper(from, to);
}

void CodeGen::VisitFextInstr(FextInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}

void CodeGen::VisitFtoUInstr(FtoUInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvttEmitHelper(from, to);
}

void CodeGen::VisitFtoSInstr(FtoSInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvttEmitHelper(from, to);
}

void CodeGen::VisitUtoFInstr(UtoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}

void CodeGen::VisitStoFInstr(StoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}


void CodeGen::VisitPtrtoIInstr(PtrtoIInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    if (from->Is<x64Reg>())
        LeaqEmitHelper(from, to);
    else if (from->As<x64Mem>()->LoadTwice())
        MovEmitHelper(from, to);
    else
        LeaqEmitHelper(from, to);
}

void CodeGen::VisitItoPtrInstr(ItoPtrInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    // LoadTwice has been set in the register allocator.
    MovEmitHelper(from, to);
}


void CodeGen::VisitIcmpInstr(IcmpInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Op1());
    auto rhs = alloc_.GetIROpMap(inst->Op2());
    auto ans = alloc_.GetIROpMap(inst->Result());
    auto issigned = inst->Op1()->Type()->As<IntType>()->IsSigned() ||
        inst->Op2()->Type()->As<IntType>()->IsSigned();

    asmfile_.EmitCmp(rhs, lhs);
    asmfile_.EmitSet(Cond2Str(inst->Cond(), issigned), ans);
    MovzEmitHelper(1, ans->Size(), ans);
}


void CodeGen::VisitFcmpInstr(FcmpInstr* inst)
{
    auto lhs = MapPossibleFloat(inst->Op1());
    auto rhs = MapPossibleFloat(inst->Op2());
    auto ans = alloc_.GetIROpMap(inst->Result());

    asmfile_.EmitUcom(rhs, lhs);
    asmfile_.EmitSet(Cond2Str(inst->Cond(), true), ans);
    MovzEmitHelper(1, ans->Size(), ans);
}


void CodeGen::VisitSelectInstr(SelectInstr* inst)
{
    auto [cond, ty] = inst->CondPair();
    auto mappedcond = MapPossibleFloat(cond);
    auto v1 = MapPossibleFloat(inst->Value1());
    auto v2 = MapPossibleFloat(inst->Value2());
    auto ans = alloc_.GetIROpMap(inst->Result());

    if (cond->Type()->Is<IntType>())
        asmfile_.EmitTest(mappedcond, mappedcond);
    else
    {
        auto regs = alloc_.NotUsedVecReg();
        if (regs.empty())
        {
            std::string label = "";
            if (fpconst_.find(0) == fpconst_.end())
            {
                label = GetLCLabel();
                asmfile_.EmitLabel(label);
                asmfile_.EmitPseudoInstr(".long", { "0" });
                fpconst_[0] = label;
            }
            else
                label = fpconst_[0];

            x64Mem zero{ 0, label };
            asmfile_.EmitUcom(&zero, mappedcond);
        }
        else
        {
            auto reg = X64Phys2RegTag(*regs.begin());
            asmfile_.EmitVarithm("xor", reg, reg, reg);
            asmfile_.EmitUcom(reg, mappedcond);
        }
    }

    if (inst->Result()->Type()->Is<IntType>())
    {
        asmfile_.EmitMov(v1, ans);
        asmfile_.EmitCMov(ty ? "e" : "ne", v2, ans);
    }
    else
    {
        // simply use branched code here
        // things like bitwise operations works fine as
        // well, but they require far more complex code,
        // so I don't use them here
        auto temp = GetTempLabel();
        VecMovEmitHelper(v1, ans);
        asmfile_.EmitJmp(ty ? "e" : "ne", temp);
        VecMovEmitHelper(v2, ans);
        asmfile_.EmitLabel(temp);
    }
}


void CodeGen::VisitPhiInstr(PhiInstr* inst)
{
    // FIXME: the implementation can't correctly solve the
    // lost-copies and phi-swap problems. With the limited
    // use of phi instructions in the code generated by the
    // front end, however, it's okay to ignore them before
    // mem2reg pass is actually implemented.

    auto curbb = asmfile_.CurBlock();
    auto mappedres = alloc_.GetIROpMap(inst->Result());

    for (auto [bb, op] : inst->GetBlockValPair())
    {
        asmfile_.SwitchBlock(bb);

        if (op->Type()->Is<IntType>())
            MovEmitHelper(alloc_.GetIROpMap(op), mappedres);
        else if (op->Type()->Is<FloatType>())
            VecMovEmitHelper(MapPossibleFloat(op), mappedres);
    }

    asmfile_.SwitchBlock(curbb);
}
