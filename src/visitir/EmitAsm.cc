#include "visitir/EmitAsm.h"
#include <climits>
#include <fmt/format.h>
#include <random>

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
    if (op->Size() == 4) return "ss";
    if (op->Size() == 8) return "sd";
    return "";
}


void EmitAsm::Output(const std::string& str)
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


void EmitAsm::EmitLabel(const std::string& label)
{
    Output(fmt::format("{}:\n", label));
}

void EmitAsm::EmitPseudoInstr(const std::string& instr)
{
    Output(fmt::format(INDENT "{}\n", instr));
}

void EmitAsm::EmitPseudoInstr(
    const std::string& instr, std::initializer_list<std::string> args)
{
    std::string line = fmt::format(INDENT "{} ", instr);
    for (auto i = args.begin(); i < args.end() - 1; ++i)
        line += fmt::format("{}, ", *i);
    line += fmt::format("{}\n", *(args.end() - 1));
    Output(line);
}


void EmitAsm::EmitCxtx(size_t size)
{
    char from = size == 4 ? 'l' : 'q';
    char to = from == 'q' ? 'o' : 'l';
    Output(fmt::format(INDENT "c{}t{}\n", from, to));
}

void EmitAsm::EmitLeaq(const x64* addr, const x64* dest)
{
    Output(fmt::format(
        INDENT "leaq {}, {}\n", addr->ToString(), dest->ToString()));
}

void EmitAsm::EmitUnary(const std::string& instr, const x64* op)
{
    Output(fmt::format(
        INDENT "{}{} {}\n", instr, GetIntTag(op), op->ToString()));
}

void EmitAsm::EmitBinary(const std::string& instr, const x64* op1, const x64* op2)
{
    Output(fmt::format(INDENT "{}{} {}, {}\n",
        instr, GetIntTag(op1), op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitVarithm(const std::string& instr, const x64* op1,
    const x64* op2, const x64* dest)
{
    auto precision = GetFltTag(op1);

    if (instr == "sqrt")
        Output(fmt::format(INDENT "{}{} {}, {}\n",
            instr, precision, op1->ToString(), dest->ToString()));
    else if (instr == "and")
        Output(fmt::format(INDENT "{}{} {}, {}, {}\n",
            instr, precision, op1->ToString(), op2->ToString(), dest->ToString()));
    else
        Output(fmt::format(INDENT "v{}{} {}, {}, {}\n",
            instr, precision, op1->ToString(), op2->ToString(), dest->ToString()));
}

void EmitAsm::EmitVcvtt(const x64* src, const x64* dest)
{
    auto from = GetFltTag(src);
    auto to = dest->Size() == 8 ? "q " : " ";

    Output(fmt::format(INDENT "vcvtt{}2si{}{}, {}\n",
        from, to, src->ToString(), dest->ToString()));
}

void EmitAsm::EmitVcvt(const x64* op1, const x64* op2, const x64* dest)
{
    auto t2 = GetFltTag(dest);
    auto suffix = op2->Size() == 8 ? "q " : " ";

    Output(fmt::format(INDENT "vcvtsi2{}{}{}, {}, {}\n",
        t2, suffix, op1->ToString(), op2->ToString(), dest->ToString()));
}

void EmitAsm::EmitUcom(const x64* op1, const x64* op2)
{
   Output(fmt::format(INDENT "vucomi{} {}, {}\n",
        GetFltTag(op1), op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitMov(const x64* src, const x64* dest)
{
    Output(fmt::format(INDENT "mov{} {}, {}\n",
        GetIntTag(src), src->ToString(), dest->ToString()));
}

void EmitAsm::EmitMov(const x64* src, RegTag tag)
{
    auto dest = x64Reg(tag, src->Size());
    Output(fmt::format(INDENT "mov{} {}, {}\n",
        GetIntTag(src), src->ToString(), dest.ToString()));
}

void EmitAsm::EmitMov(RegTag tag, const x64* dest)
{
    auto src = x64Reg(tag, dest->Size());
    Output(fmt::format(INDENT "mov{} {}, {}\n",
        GetIntTag(dest), src.ToString(), dest->ToString()));
}

void EmitAsm::EmitMovz(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    Output(fmt::format(INDENT "movz{}{} {}, {}\n",
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

    Output(fmt::format(
        INDENT "movz{}{} {}, {}\n", tfrm, tto, sfrm, sto));

    const_cast<x64*>(op)->Size() = original;
}

void EmitAsm::EmitMovs(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    Output(fmt::format(INDENT "movs{}{} {}, {}\n",
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

    Output(fmt::format(
        INDENT "movs{}{} {}, {}\n", tfrm, tto, sfrm, sto));

    const_cast<x64*>(op)->Size() = original;
}

void EmitAsm::EmitVmov(const x64* src, const x64* dest)
{
    Output(fmt::format(INDENT "vmov{} {}, {}\n",
        GetFltTag(src), src->ToString(), dest->ToString()));
}


void EmitAsm::EmitPop(const x64* dest)
{
    Output(fmt::format(INDENT "pop{} {}\n",
        GetIntTag(dest), dest->ToString()));
}

void EmitAsm::EmitPop(RegTag tag, size_t size)
{
    x64Reg reg{ tag, size };
    EmitPop(&reg);
}

void EmitAsm::EmitPush(const x64* dest)
{
    Output(fmt::format(INDENT "push{} {}\n",
        GetIntTag(dest), dest->ToString()));
}

void EmitAsm::EmitPush(RegTag tag, size_t size)
{
    x64Reg reg{ tag, size };
    EmitPush(&reg);
}


void EmitAsm::EmitCall(const std::string& func)
{
    Output(fmt::format(INDENT "call {}\n", func));
}

void EmitAsm::EmitCall(const x64* func)
{
    Output(fmt::format(INDENT "call *{}\n", func->ToString()));
}

void EmitAsm::EmitRet()
{
    Output(fmt::format(INDENT "ret\n"));
}


void EmitAsm::EmitJmp(const std::string& cond, const std::string& label)
{
    Output(fmt::format(INDENT "j{} {}\n",
        cond.empty() ? "mp" : cond, label));
}

void EmitAsm::EmitCMov(const std::string& cond, const x64* op1, const x64* op2)
{
    Output(fmt::format(INDENT "cmov{} {}, {}\n",
        cond, op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitCmp(const x64* op1, const x64* op2)
{
    Output(fmt::format(INDENT "cmp{} {}, {}\n",
        GetIntTag(op1), op1->ToString(), op2->ToString()));
}

void EmitAsm::EmitCmp(const x64* op1, unsigned long c)
{
    Output(fmt::format(INDENT "cmp{} {}, ${}\n",
        GetIntTag(op1), op1->ToString(), std::to_string(c)));
}

void EmitAsm::EmitTest(const x64* op1, const x64* op2)
{
    Output(fmt::format(INDENT "test{} {}, {}\n",
        GetIntTag(op1), op1->ToString(), op2->ToString()));
}

void EmitAsm::EmitTest(const x64* op1, unsigned long c)
{
    Output(fmt::format(INDENT "test{} {}, ${}\n",
        GetIntTag(op1), op1->ToString(), std::to_string(c)));
}

void EmitAsm::EmitSet(const std::string& cond, const x64* dest)
{
    Output(fmt::format(INDENT "set{} {}\n",
        cond, dest->ToString()));
}
