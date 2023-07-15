#include "visitir/EmitAsm.h"
#include "visitir/x64.h"
#include <fmt/format.h>

#define INDENT "    "


static char GetIntTag(const x64* op)
{
    if (op->Size() == 1) return 'b';
    if (op->Size() == 2) return 'w';
    if (op->Size() == 4) return 'l';
    if (op->Size() == 8) return 'q';
    return '\0';
}

static std::string GetFltTag(const x64* op)
{
    if (op->Size() == 4)
        return "ss";
    if (op->Size() == 8 || op->Size() == 16)
        return "sd";
    return "";
}


void EmitAsm::EmitInstr(const std::string& str)
{
    if (write2file_) file_ << str;
    else blks_[curblk_].push_back(str);
}


void EmitAsm::EnterBlock(const BasicBlock* bb)
{
    curblk_ = bb;
    blkindexes_.push_back(bb);
}

void EmitAsm::SwitchBlock(const BasicBlock* bb)
{
    curblk_ = bb;
}

void EmitAsm::Dump2File()
{
    for (auto bb : blkindexes_)
        for (auto s : blks_[bb])
            file_ << s;

    blks_.clear();
    blkindexes_.clear();
    write2file_ = true;
    labelindex_ = 0;
}


void EmitAsm::EmitBlankLine()
{
    EmitInstr("\n");
}

void EmitAsm::EmitLabel(const std::string& label)
{
    EmitInstr(fmt::format("{}:\n", label));
}

void EmitAsm::EmitPseudoInstr(const std::string& instr)
{
    EmitInstr(fmt::format(INDENT "{}\n", instr));
}

void EmitAsm::EmitPseudoInstr(
    const std::string& instr, std::initializer_list<std::string> args)
{
    std::string line = fmt::format(INDENT "{} ", instr);
    for (auto i = args.begin(); i < args.end() - 1; ++i)
        line += fmt::format("{}, ", *i);
    line += fmt::format("{}\n", *(args.end() - 1));
    EmitInstr(line);
}


void EmitAsm::EmitCxtx(size_t size)
{
    char from = size == 4 ? 'l' : 'q';
    char to = from == 'q' ? 'o' : 'l';
    EmitInstr(fmt::format(INDENT "c{}t{}\n", from, to));
}

void EmitAsm::EmitLeaq(const x64* addr, const x64* dest)
{
    EmitInstr(fmt::format(
        INDENT "leaq {}, {}\n", addr->ToString(), dest->ToString()));
}

void EmitAsm::EmitLeaq(const x64* addr, RegTag dest)
{
    x64Reg reg{ dest };
    EmitLeaq(addr, &reg);
}

void EmitAsm::EmitUnary(const std::string& instr, const x64* op)
{
    EmitInstr(fmt::format(
        INDENT "{}{} {}\n", instr, GetIntTag(op), op->ToString()));
}

void EmitAsm::EmitBinary(const std::string& instr, unsigned long imm, const x64* op)
{
    EmitInstr(fmt::format(INDENT "{}{} ${}, {}\n",
        instr, GetIntTag(op), std::to_string(imm), op->ToString()));
}

void EmitAsm::EmitBinary(const std::string& instr, const x64* op1, const x64* op2)
{
    EmitInstr(fmt::format(INDENT "{}{} {}, {}\n",
        instr, GetIntTag(op1), op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitVarithm(const std::string& instr, const x64* op1,
    const x64* op2, const x64* dest)
{
    auto precision = GetFltTag(op1);

    if (instr == "sqrt")
        EmitInstr(fmt::format(INDENT "{}{} {}, {}\n",
            instr, precision, op1->ToString(), dest->ToString()));
    else if (instr == "and")
        EmitInstr(fmt::format(INDENT "{}{} {}, {}, {}\n",
            instr, precision, op1->ToString(), op2->ToString(), dest->ToString()));
    else
        EmitInstr(fmt::format(INDENT "v{}{} {}, {}, {}\n",
            instr, precision, op1->ToString(), op2->ToString(), dest->ToString()));
}

void EmitAsm::EmitVarithm(const std::string& instr, RegTag op1, RegTag op2, RegTag op3)
{
    x64Reg r1{ op1 }, r2{ op2 }, r3{ op3 };
    EmitVarithm(instr, &r1, &r2, &r3);
}


void EmitAsm::EmitVcvtt(const x64* src, const x64* dest)
{
    auto from = GetFltTag(src);
    auto to = GetIntTag(dest);

    EmitInstr(fmt::format(INDENT "vcvtt{}2si{}{}, {}\n",
        from, to, src->ToString(), dest->ToString()));
}

void EmitAsm::EmitVcvtt(const x64* op1, RegTag op2)
{
    x64Reg reg{ op2 };
    EmitVcvtt(op1, &reg);
}


void EmitAsm::EmitVcvt(const x64* op1, const x64* dest)
{
    auto from = GetFltTag(op1);
    auto to = GetFltTag(dest);
    auto sdest = dest->ToString();

    EmitInstr(fmt::format(INDENT "vcvt{}2{} {}, {}, {}\n",
        from, to, op1->ToString(), sdest, sdest));
}

void EmitAsm::EmitVcvt(const x64* op1, RegTag op2)
{
    x64Reg reg{ op2 };
    EmitVcvt(op1, &reg);
}

void EmitAsm::EmitVcvtsi(const x64* op1, const x64* dest)
{
    auto t2 = GetFltTag(dest);
    auto suffix = GetIntTag(op1);
    auto sdest = dest->ToString();

    EmitInstr(fmt::format(INDENT "vcvtsi2{}{}{}, {}, {}\n",
        t2, suffix, op1->ToString(), sdest, sdest));
}

void EmitAsm::EmitVcvtsi(const x64* op1, RegTag op2)
{
    x64Reg reg{ op2 };
    EmitVcvtsi(op1, &reg);
}


void EmitAsm::EmitUcom(const x64* op1, const x64* op2)
{
   EmitInstr(fmt::format(INDENT "vucomi{} {}, {}\n",
        GetFltTag(op1), op1->ToString(), op2->ToString()));
}

void EmitAsm::EmitUcom(const x64* op1, RegTag op2)
{
    x64Reg reg{ op2 };
    EmitUcom(op1, &reg);
}


void EmitAsm::EmitMov(const x64* src, const x64* dest)
{
    EmitInstr(fmt::format(INDENT "mov{} {}, {}\n",
        GetIntTag(dest), src->ToString(), dest->ToString()));
}

void EmitAsm::EmitMov(const x64* src, RegTag tag)
{
    auto dest = x64Reg(tag, src->Size());
    EmitInstr(fmt::format(INDENT "mov{} {}, {}\n",
        GetIntTag(src), src->ToString(), dest.ToString()));
}

void EmitAsm::EmitMov(RegTag tag, const x64* dest)
{
    auto src = x64Reg(tag, dest->Size());
    EmitInstr(fmt::format(INDENT "mov{} {}, {}\n",
        GetIntTag(dest), src.ToString(), dest->ToString()));
}

void EmitAsm::EmitMov(RegTag tag, long offset)
{
    auto src = x64Reg(tag, 8);
    auto rsp = x64Mem(8, offset, RegTag::rsp, RegTag::none, 0);
    EmitInstr(fmt::format(
        INDENT "movq {}, {}\n", src.ToString(), rsp.ToString()));
}

void EmitAsm::EmitMovz(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    EmitInstr(fmt::format(INDENT "movz{}{} {}, {}\n",
        from, to, src->ToString(), dest->ToString()));
}

void EmitAsm::EmitMovz(size_t from, size_t to, const x64* op)
{
    auto original = op->Size();

    const_cast<x64*>(op)->Size() = from;
    auto sfrm = op->ToString();
    auto tfrm = GetIntTag(op);

    const_cast<x64*>(op)->Size() = to;
    auto sto = op->ToString();
    auto tto = GetIntTag(op);

    EmitInstr(fmt::format(
        INDENT "movz{}{} {}, {}\n", tfrm, tto, sfrm, sto));

    const_cast<x64*>(op)->Size() = original;
}

void EmitAsm::EmitMovs(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    EmitInstr(fmt::format(INDENT "movs{}{} {}, {}\n",
        from, to, src->ToString(), dest->ToString()));
}

void EmitAsm::EmitMovs(size_t from, size_t to, const x64* op)
{
    auto original = op->Size();

    const_cast<x64*>(op)->Size() = from;
    auto sfrm = op->ToString();
    auto tfrm = GetIntTag(op);

    const_cast<x64*>(op)->Size() = to;
    auto sto = op->ToString();
    auto tto = GetIntTag(op);

    EmitInstr(fmt::format(
        INDENT "movs{}{} {}, {}\n", tfrm, tto, sfrm, sto));

    const_cast<x64*>(op)->Size() = original;
}


void EmitAsm::EmitVmov(const x64* src, const x64* dest)
{
    EmitInstr(fmt::format(INDENT "vmov{} {}, {}\n",
        GetFltTag(src), src->ToString(), dest->ToString()));
}

void EmitAsm::EmitVmov(const x64* src, RegTag dest)
{
    auto destreg = x64Reg(dest, src->Size());
    EmitInstr(fmt::format(INDENT "vmov{} {}, {}\n",
        GetFltTag(src), src->ToString(), destreg.ToString()));
}

void EmitAsm::EmitVmov(RegTag src, const x64* dest)
{
    auto srcreg = x64Reg(src, dest->Size());
    EmitInstr(fmt::format(INDENT "vmov{} {}, {}\n",
        GetFltTag(dest), srcreg.ToString(), dest->ToString()));
}

void EmitAsm::EmitVmovap(const x64Reg* src, const x64Reg* dest)
{
    EmitInstr(fmt::format(INDENT "vmovap{} {}, {}\n",
        GetFltTag(src)[1], src->ToString(), dest->ToString()));
}

void EmitAsm::EmitVmovap(const x64Reg* src, RegTag dest)
{
    x64Reg reg{ dest, src->Size() };
    EmitVmovap(src, &reg);
}

void EmitAsm::EmitVmovap(RegTag src, const x64Reg* dest)
{
    x64Reg reg{ src, dest->Size() };
    EmitVmovap(&reg, dest);
}

void EmitAsm::EmitVmov(RegTag src, long offset)
{
    x64Reg reg{ src, 8 };
    auto rsp = x64Mem(8, offset, RegTag::rsp, RegTag::none, 0);
    EmitVmov(&reg, &rsp);
}


void EmitAsm::EmitPop(const x64* dest)
{
    EmitInstr(fmt::format(INDENT "pop{} {}\n",
        GetIntTag(dest), dest->ToString()));
}

void EmitAsm::EmitPop(RegTag tag, size_t size)
{
    x64Reg reg{ tag, size };
    EmitPop(&reg);
}

void EmitAsm::EmitPush(const x64* dest)
{
    EmitInstr(fmt::format(INDENT "push{} {}\n",
        GetIntTag(dest), dest->ToString()));
}

void EmitAsm::EmitPush(RegTag tag, size_t size)
{
    x64Reg reg{ tag, size };
    EmitPush(&reg);
}


void EmitAsm::EmitCall(const std::string& func)
{
    EmitInstr(fmt::format(INDENT "call {}\n", func));
}

void EmitAsm::EmitCall(const x64* func)
{
    EmitInstr(fmt::format(INDENT "call *{}\n", func->ToString()));
}

void EmitAsm::EmitLeave()
{
    EmitInstr(INDENT "leave\n");
}

void EmitAsm::EmitRet()
{
    EmitInstr(INDENT "ret\n");
}


void EmitAsm::EmitJmp(const std::string& cond, const std::string& label)
{
    EmitInstr(fmt::format(INDENT "j{} {}\n",
        cond.empty() ? "mp" : cond, label));
}

void EmitAsm::EmitCMov(const std::string& cond, const x64* op1, const x64* op2)
{
    EmitInstr(fmt::format(INDENT "cmov{} {}, {}\n",
        cond, op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitCmp(const x64* op1, const x64* op2)
{
    EmitInstr(fmt::format(INDENT "cmp{} {}, {}\n",
        GetIntTag(op1), op1->ToString(), op2->ToString()));
}

void EmitAsm::EmitCmp(RegTag op1, const x64* op2)
{
    x64Reg reg{ op1, op2->Size() };
    EmitCmp(&reg, op2);
}

void EmitAsm::EmitCmp(const x64* op1, unsigned long c)
{
    EmitInstr(fmt::format(INDENT "cmp{} {}, ${}\n",
        GetIntTag(op1), op1->ToString(), std::to_string(c)));
}

void EmitAsm::EmitTest(const x64* op1, const x64* op2)
{
    EmitInstr(fmt::format(INDENT "test{} {}, {}\n",
        GetIntTag(op1), op1->ToString(), op2->ToString()));
}

void EmitAsm::EmitTest(RegTag op1, const x64* op2)
{
    x64Reg reg{ op1, op2->Size() };
    EmitTest(&reg, op2);
}

void EmitAsm::EmitTest(const x64* op1, unsigned long c)
{
    EmitInstr(fmt::format(INDENT "test{} {}, ${}\n",
        GetIntTag(op1), op1->ToString(), std::to_string(c)));
}

void EmitAsm::EmitSet(const std::string& cond, const x64* dest)
{
    EmitInstr(fmt::format(INDENT "set{} {}\n",
        cond, dest->ToString()));
}

void EmitAsm::EmitSet(const std::string& cond, RegTag dest)
{
    x64Reg reg{ dest, 1 };
    EmitInstr(fmt::format(INDENT "set{} {}\n",
        cond, reg.ToString()));
}
