#include "visitir/EmitAsm.h"
#include <random>
#include <climits>

#define INDENT "    "
#define TEMP_PREFIX "@__Ginkgo_temp_"


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


void EmitAsm::EmitLeaq(
    const std::string& addr, const std::string& dest)
{
    fprintf(file_, INDENT "leaq %s, %s\n", addr.c_str(), dest.c_str());
}

void EmitAsm::EmitUnary(
    char suffix, const std::string& instr, const std::string& op)
{
    fprintf(file_, INDENT "%s%c %s\n", instr.c_str(), suffix, op.c_str());
}

void EmitAsm::EmitBinary(char suffix, const std::string& instr,
    const std::string& op1, const std::string& op2)
{
    fprintf(file_, INDENT "%s%c %s, %s\n",
        instr.c_str(), suffix, op1.c_str(), op2.c_str());
}


void EmitAsm::EmitVarithm(const std::string& instr, const std::string& precison,
    const std::string& op1, const std::string& op2, const std::string& dest)
{
    if (instr == "sqrt")
        fprintf(file_, INDENT "%s%s %s, %s\n",
            instr.c_str(), precison.c_str(), op1.c_str(), dest.c_str());
    else if (instr == "and")
        fprintf(file_, INDENT "%s%s %s, %s, %s\n",
            instr.c_str(), precison.c_str(), op1.c_str(), op2.c_str(), dest.c_str());
    else
        fprintf(file_, INDENT "v%s%s %s, %s, %s\n",
            instr.c_str(), precison.c_str(), op1.c_str(), op2.c_str(), dest.c_str());
}

void EmitAsm::EmitVcvtt(const std::string& from, const std::string& to,
    const std::string& src, const std::string& dest)
{
    fprintf(file_, INDENT "vcvtt%s2%s %s, %s\n",
        from.c_str(), to.c_str(), src.c_str(), dest.c_str());
}

void EmitAsm::EmitVcvt(const std::string& from, const std::string& to,
    const std::string& op1, const std::string& op2, const std::string& dest)
{
    fprintf(file_, INDENT "vcvt%s2%s %s, %s, %s\n",
        from.c_str(), to.c_str(), op1.c_str(), op2.c_str(), dest.c_str());
}

void EmitAsm::EmitUcom(
    const std::string& precison, const std::string& op1, const std::string& op2)
{
    fprintf(file_, INDENT "vucomi%s %s, %s\n",
        precison.c_str(), op1.c_str(), op2.c_str());
}


void EmitAsm::EmitMov(char suffix, const std::string& src, const std::string& dest)
{
    fprintf(file_, INDENT "mov%c %s, %s\n", suffix, src.c_str(), dest.c_str());
}

void EmitAsm::EmitMovz(
    char from, char to, const std::string& src, const std::string& dest)
{
    fprintf(file_, INDENT "movz%c%c %s, %s\n", from, to, src.c_str(), dest.c_str());
}

void EmitAsm::EmitMovs(
    char from, char to, const std::string& src, const std::string& dest)
{
    fprintf(file_, INDENT "movs%c%c %s, %s\n", from, to, src.c_str(), dest.c_str());
}

void EmitAsm::EmitVmov(
    const std::string& suffix, const std::string& src, const std::string& dest)
{
    fprintf(file_, INDENT "vmov%s %s, %s\n", suffix.c_str(), src.c_str(), dest.c_str());
}


void EmitAsm::EmitPop(char suffix, const std::string& dest)
{
    fprintf(file_, INDENT "pop%c %s\n", suffix, dest.c_str());
}

void EmitAsm::EmitPush(char suffix, const std::string& dest)
{
    fprintf(file_, INDENT "push%c %s\n", suffix, dest.c_str());
}


void EmitAsm::EmitCall(const std::string& func)
{
    fprintf(file_, INDENT "call %s\n", func.c_str());
}

void EmitAsm::EmitRet()
{
    fprintf(file_, INDENT "ret\n");
}


void EmitAsm::EmitJmp(const std::string& condition, const std::string& label)
{
    fprintf(file_, INDENT "j%s %s\n",
        condition.empty() ? "mp" : condition.c_str(), label.c_str());
}

void EmitAsm::EmitCMov(
    const std::string& condition, const std::string& op1, const std::string& op2)
{
    fprintf(file_, INDENT "cmov%s %s, %s\n", condition.c_str(), op1.c_str(), op2.c_str());
}


void EmitAsm::EmitCmp(char suffix, const std::string& op1, const std::string& op2)
{
    fprintf(file_, INDENT "cmp%c %s, %s\n", suffix, op1.c_str(), op2.c_str());
}

void EmitAsm::EmitTest(char suffix, const std::string& op1, const std::string& op2)
{
    fprintf(file_, INDENT "test%c %s, %s\n", suffix, op1.c_str(), op2.c_str());
}

void EmitAsm::EmitSet(const std::string& condition, const std::string& dest)
{
    fprintf(file_, INDENT "set%s %s\n", condition.c_str(), dest.c_str());
}
