#include "visitir/EmitAsm.h"
#include <random>
#include <climits>

#define INDENT "    "
#define TEMP_PREFIX "@__Ginkgo_temp_"


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


EmitAsm::EmitAsm()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, INT32_MAX);

    filename_ = TEMP_PREFIX + std::to_string(dis(gen)) + ".s";
    file_ = fopen(filename_.c_str(), "w+");
}

EmitAsm::EmitAsm(const std::string& filename) :
    filename_(filename), file_(fopen(filename.c_str(), "w"))
{
}

EmitAsm::~EmitAsm()
{
    fclose(file_);
}


void EmitAsm::EmitLabel(const std::string& label)
{
    fprintf(file_, "%s:\n", label.c_str());
}


void EmitAsm::EmitPseudoInstr(const std::string& instr)
{
    fprintf(file_, INDENT "%s\n", instr.c_str());
}

void EmitAsm::EmitPseudoInstr(
    const std::string& instr, std::initializer_list<std::string> args)
{
    fprintf(file_, INDENT "%s ", instr.c_str());
    for (auto i = args.begin(); i < args.end() - 1; ++i)
        fprintf(file_, "%s, ", i->c_str());
    fprintf(file_, "%s\n", (args.end() - 1)->c_str());
}


void EmitAsm::EmitCxtx(const x64Reg* rax)
{
    char from = GetIntTag(rax);
    char to = from == 'q' ? 'o' : 'l';
    fprintf(file_, INDENT "c%ct%c\n", from, to);
}

void EmitAsm::EmitLeaq(const x64* addr, const x64* dest)
{
    fprintf(file_, INDENT "leaq %s, %s\n",
        addr->ToString().c_str(), dest->ToString().c_str());
}

void EmitAsm::EmitUnary(const std::string& instr, const x64* op)
{
    fprintf(file_, INDENT "%s%c %s\n",
        instr.c_str(), GetIntTag(op), op->ToString().c_str());
}

void EmitAsm::EmitBinary(const std::string& instr, const x64* op1, const x64* op2)
{
    fprintf(file_, INDENT "%s%c %s, %s\n", instr.c_str(),
        GetIntTag(op1), op1->ToString().c_str(), op2->ToString().c_str());
}


void EmitAsm::EmitVarithm(const std::string& instr, const x64* op1,
    const x64* op2, const x64* dest)
{
    auto precision = GetFltTag(op1);

    if (instr == "sqrt")
        fprintf(file_, INDENT "%s%s %s, %s\n", instr.c_str(),
            precision.c_str(), op1->ToString().c_str(), dest->ToString().c_str());
    else if (instr == "and")
        fprintf(file_, INDENT "%s%s %s, %s, %s\n", instr.c_str(),
            precision.c_str(), op1->ToString().c_str(), op2->ToString().c_str(), dest->ToString().c_str());
    else
        fprintf(file_, INDENT "v%s%s %s, %s, %s\n", instr.c_str(),
            precision.c_str(), op1->ToString().c_str(), op2->ToString().c_str(), dest->ToString().c_str());
}

void EmitAsm::EmitVcvtt(const x64* src, const x64* dest)
{
    auto from = GetFltTag(src);
    auto to = dest->Size() == 8 ? "q " : " ";

    fprintf(file_, INDENT "vcvtt%s2si%s%s, %s\n",
        from.c_str(), to, src->ToString().c_str(), dest->ToString().c_str());
}

void EmitAsm::EmitVcvt(const x64* op1, const x64* op2, const x64* dest)
{
    auto t2 = GetFltTag(dest);
    auto suffix = op2->Size() == 8 ? "q " : " ";

    fprintf(file_, INDENT "vcvtsi2%s%s%s, %s, %s\n", t2.c_str(), suffix,
        op1->ToString().c_str(), op2->ToString().c_str(), dest->ToString().c_str());
}

void EmitAsm::EmitUcom(const x64* op1, const x64* op2)
{
    fprintf(file_, INDENT "vucomi%s %s, %s\n",
        GetFltTag(op1).c_str(), op1->ToString().c_str(), op2->ToString().c_str());
}


void EmitAsm::EmitMov(const x64* src, const x64* dest)
{
    fprintf(file_, INDENT "mov%c %s, %s\n",
        GetIntTag(src), src->ToString().c_str(), dest->ToString().c_str());
}

void EmitAsm::EmitMovz(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    fprintf(file_, INDENT "movz%c%c %s, %s\n", from, to,
        src->ToString().c_str(), dest->ToString().c_str());
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

    fprintf(file_, INDENT "movz%c%c %s, %s\n",
        tfrm, tto, sfrm.c_str(), sto.c_str());

    const_cast<x64*>(op)->Size() = original;
}

void EmitAsm::EmitMovs(const x64* src, const x64* dest)
{
    char from = GetIntTag(src);
    char to = GetIntTag(dest);
    fprintf(file_, INDENT "movs%c%c %s, %s\n",
        from, to, src->ToString().c_str(), dest->ToString().c_str());
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

    fprintf(file_, INDENT "movs%c%c %s, %s\n",
        tfrm, tto, sfrm.c_str(), sto.c_str());

    const_cast<x64*>(op)->Size() = original;
}

void EmitAsm::EmitVmov(const x64* src, const x64* dest)
{
    fprintf(file_, INDENT "vmov%s %s, %s\n",
        GetFltTag(src).c_str(), src->ToString().c_str(), dest->ToString().c_str());
}


void EmitAsm::EmitPop(const x64Reg* dest)
{
    fprintf(file_, INDENT "pop%c %s\n",
        GetIntTag(dest), dest->ToString().c_str());
}

void EmitAsm::EmitPush(const x64Reg* dest)
{
    fprintf(file_, INDENT "push%c %s\n",
        GetIntTag(dest), dest->ToString().c_str());
}


void EmitAsm::EmitCall(const std::string& func)
{
    fprintf(file_, INDENT "call %s\n", func.c_str());
}

void EmitAsm::EmitCall(const x64* func)
{
    fprintf(file_, INDENT "call *%s\n", func->ToString().c_str());
}

void EmitAsm::EmitRet()
{
    fprintf(file_, INDENT "ret\n");
}


void EmitAsm::EmitJmp(const std::string& cond, const std::string& label)
{
    fprintf(file_, INDENT "j%s %s\n",
        cond.empty() ? "mp" : cond.c_str(), label.c_str());
}

void EmitAsm::EmitCMov(const std::string& cond, const x64* op1, const x64* op2)
{
    fprintf(file_, INDENT "cmov%s %s, %s\n",
        cond.c_str(), op1->ToString().c_str(), op2->ToString().c_str());
}


void EmitAsm::EmitCmp(const x64* op1, const x64* op2)
{
    fprintf(file_, INDENT "cmp%c %s, %s\n",
        GetIntTag(op1), op1->ToString().c_str(), op2->ToString().c_str());
}

void EmitAsm::EmitCmp(const x64* op1, unsigned long c)
{
    fprintf(file_, INDENT "cmp%c %s, $%s\n",
        GetIntTag(op1), op1->ToString().c_str(), std::to_string(c));
}

void EmitAsm::EmitTest(const x64* op1, const x64* op2)
{
    fprintf(file_, INDENT "test%c %s, %s\n",
        GetIntTag(op1), op1->ToString().c_str(), op2->ToString().c_str());
}

void EmitAsm::EmitSet(const std::string& cond, const x64* dest)
{
    fprintf(file_, INDENT "set%s %s\n",
        cond.c_str(), dest->ToString().c_str());
}
