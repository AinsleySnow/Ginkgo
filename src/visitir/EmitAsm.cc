#include "visitir/EmitAsm.h"
#include <climits>
#include <fmt/format.h>
#include <random>

#define INDENT "    "


char EmitAsm::GetIntTag(const x64* op) const
{
    if (op->Size() == 1) return 'b';
    if (op->Size() == 2) return 'w';
    if (op->Size() == 4) return 'l';
    if (op->Size() == 8) return 'q';
    return '\0';
}

std::string EmitAsm::GetFltTag(const x64* op) const
{
    if (op->Size() == 4) return "ss";
    if (op->Size() == 8) return "sd";
    return "";
}


EmitAsm::EmitAsm(const std::string& filename) : filename_(filename)
{
    file_.open(filename);
}

EmitAsm::~EmitAsm()
{
    file_.close();
}


void EmitAsm::EnterBlock(int i, const BasicBlock* bb)
{
    curblk_ = bb;
    index_.emplace(index_.begin() + i, bb);
}


void EmitAsm::WriteFile()
{
    for (auto bb : index_)
        for (auto s : blks_[bb])
            file_ << s;
}


void EmitAsm::EmitLabel(const std::string& label)
{
    blks_[curblk_].push_back(fmt::format("{}:\n", label));
}


void EmitAsm::EmitPseudoInstr(const std::string& instr)
{
    blks_[curblk_].push_back(fmt::format(INDENT "{}\n", instr));
}

void EmitAsm::EmitPseudoInstr(
    const std::string& instr, std::initializer_list<std::string> args)
{
    std::string line = fmt::format(INDENT "{} ", instr);
    for (auto i = args.begin(); i < args.end() - 1; ++i)
        line += fmt::format("{}, ", *i);
    line += fmt::format("{}\n", *(args.end() - 1));
    blks_[curblk_].push_back(line);
}


void EmitAsm::EmitCxtx(const x64Reg* rax)
{
    char from = GetIntTag(rax);
    char to = from == 'q' ? 'o' : 'l';
    blks_[curblk_].push_back(fmt::format(INDENT "c{}t{}\n", from, to));
}

void EmitAsm::EmitLeaq(const x64* addr, const x64* dest)
{
    blks_[curblk_].push_back(fmt::format(
        INDENT "leaq {}, {}\n", addr->ToString(), dest->ToString()));
}

void EmitAsm::EmitUnary(const std::string& instr, const x64* op)
{
    blks_[curblk_].push_back(fmt::format(
        INDENT "{}{} {}\n", instr, GetIntTag(op), op->ToString()));
}

void EmitAsm::EmitBinary(const std::string& instr, const x64* op1, const x64* op2)
{
    blks_[curblk_].push_back(fmt::format(INDENT "{}{} {}, {}\n",
        instr, GetIntTag(op1), op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitVarithm(const std::string& instr, const x64* op1,
    const x64* op2, const x64* dest)
{
    auto precision = GetFltTag(op1);

    if (instr == "sqrt")
        blks_[curblk_].push_back(fmt::format(INDENT "{}{} {}, {}\n",
            instr, precision, op1->ToString(), dest->ToString()));
    else if (instr == "and")
        blks_[curblk_].push_back(fmt::format(INDENT "{}{} {}, {}, {}\n",
            instr, precision, op1->ToString(), op2->ToString(), dest->ToString()));
    else
        blks_[curblk_].push_back(fmt::format(INDENT "v{}{} {}, {}, {}\n",
            instr, precision, op1->ToString(), op2->ToString(), dest->ToString()));
}

void EmitAsm::EmitVcvtt(const x64* src, const x64* dest)
{
    auto from = GetFltTag(src);
    auto to = dest->Size() == 8 ? "q " : " ";

    blks_[curblk_].push_back(fmt::format(INDENT "vcvtt{}2si{}{}, {}\n",
        from, to, src->ToString(), dest->ToString()));
}

void EmitAsm::EmitVcvt(const x64* op1, const x64* op2, const x64* dest)
{
    auto t2 = GetFltTag(dest);
    auto suffix = op2->Size() == 8 ? "q " : " ";

    blks_[curblk_].push_back(fmt::format(INDENT "vcvtsi2{}{}{}, {}, {}\n",
        t2, suffix, op1->ToString(), op2->ToString(), dest->ToString()));
}

void EmitAsm::EmitUcom(const x64* op1, const x64* op2)
{
   blks_[curblk_].push_back(fmt::format(INDENT "vucomi{} {}, {}\n",
        GetFltTag(op1), op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitMov(const x64* src, const x64* dest)
{
    blks_[curblk_].push_back(fmt::format(INDENT "mov{} {}, {}\n",
        GetIntTag(src), src->ToString(), dest->ToString()));
}

void EmitAsm::EmitMovz(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    blks_[curblk_].push_back(fmt::format(INDENT "movz{}{} {}, {}\n",
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

    blks_[curblk_].push_back(fmt::format(
        INDENT "movz{}{} {}, {}\n", tfrm, tto, sfrm, sto));

    const_cast<x64*>(op)->Size() = original;
}

void EmitAsm::EmitMovs(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    blks_[curblk_].push_back(fmt::format(INDENT "movs{}{} {}, {}\n",
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

    blks_[curblk_].push_back(fmt::format(
        INDENT "movs{}{} {}, {}\n", tfrm, tto, sfrm, sto));

    const_cast<x64*>(op)->Size() = original;
}

void EmitAsm::EmitVmov(const x64* src, const x64* dest)
{
    blks_[curblk_].push_back(fmt::format(INDENT "vmov{} {}, {}\n",
        GetFltTag(src), src->ToString(), dest->ToString()));
}


void EmitAsm::EmitPop(const x64Reg* dest)
{
    blks_[curblk_].push_back(fmt::format(INDENT "pop{} {}\n",
        GetIntTag(dest), dest->ToString()));
}

void EmitAsm::EmitPush(const x64Reg* dest)
{
    blks_[curblk_].push_back(fmt::format(INDENT "push{} {}\n",
        GetIntTag(dest), dest->ToString()));
}


void EmitAsm::EmitCall(const std::string& func)
{
    blks_[curblk_].push_back(fmt::format(INDENT "call {}\n", func));
}

void EmitAsm::EmitCall(const x64* func)
{
    blks_[curblk_].push_back(fmt::format(INDENT "call *{}\n", func->ToString()));
}

void EmitAsm::EmitRet()
{
    blks_[curblk_].push_back(fmt::format(INDENT "ret\n"));
}


void EmitAsm::EmitJmp(const std::string& cond, const std::string& label)
{
    blks_[curblk_].push_back(fmt::format(INDENT "j{} {}\n",
        cond.empty() ? "mp" : cond, label));
}

void EmitAsm::EmitCMov(const std::string& cond, const x64* op1, const x64* op2)
{
    blks_[curblk_].push_back(fmt::format(INDENT "cmov{} {}, {}\n",
        cond, op1->ToString(), op2->ToString()));
}


void EmitAsm::EmitCmp(const x64* op1, const x64* op2)
{
    blks_[curblk_].push_back(fmt::format(INDENT "cmp{} {}, {}\n",
        GetIntTag(op1), op1->ToString(), op2->ToString()));
}

void EmitAsm::EmitCmp(const x64* op1, unsigned long c)
{
    blks_[curblk_].push_back(fmt::format(INDENT "cmp{} {}, ${}\n",
        GetIntTag(op1), op1->ToString(), std::to_string(c)));
}

void EmitAsm::EmitTest(const x64* op1, const x64* op2)
{
    blks_[curblk_].push_back(fmt::format(INDENT "test{} {}, {}\n",
        GetIntTag(op1), op1->ToString(), op2->ToString()));
}

void EmitAsm::EmitTest(const x64* op1, unsigned long c)
{
    blks_[curblk_].push_back(fmt::format(INDENT "test{} {}, ${}\n",
        GetIntTag(op1), op1->ToString(), std::to_string(c)));
}

void EmitAsm::EmitSet(const std::string& cond, const x64* dest)
{
    blks_[curblk_].push_back(fmt::format(INDENT "set{} {}\n",
        cond, dest->ToString()));
}
