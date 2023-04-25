#ifndef _EMIT_ASM_H_
#define _EMIT_ASM_H_

#include <cstdio>
#include <initializer_list>
#include <string>


class EmitAsm
{
public:
    EmitAsm();
    EmitAsm(const std::string&);
    ~EmitAsm();

    std::string AsmName() const { return filename_; }

    void EmitLabel(const std::string&);
    void EmitPseudoInstr(const std::string&);
    void EmitPseudoInstr(
        const std::string&, std::initializer_list<std::string>);

    void EmitCxtx(const x64Reg* rax);
    void EmitLeaq(const x64* addr, const x64* dest);
    void EmitUnary(const std::string& instr, const x64* op);
    void EmitBinary(const std::string& instr, const x64* op1, const x64* op2);

    void EmitVarithm(const std::string& instr, const x64* op1,
        const x64* op2, const x64* dest);
    void EmitVcvtt(const x64* src, const x64* dest);
    void EmitVcvt(const x64* op1, const x64* op2, const x64* dest);
    void EmitUcom(const x64* op1, const x64* op2);

    void EmitMov(const x64* src, const x64* dest);
    void EmitMovz(const x64* src, const x64* dest);
    void EmitMovz(size_t from, size_t to, const x64* op);
    void EmitMovs(const x64* src, const x64* dest);
    void EmitMovs(size_t from, size_t to, const x64* op);
    void EmitVmov(const x64* src, const x64* dest);

    void EmitPop(const x64Reg* dest);
    void EmitPush(const x64Reg* dest);

    void EmitCall(const std::string&);
    void EmitCall(const x64* func);
    void EmitRet();

    void EmitJmp(const std::string&, const std::string&);
    void EmitCMov(const std::string& cond, const x64* op1, const x64* op2);

    void EmitCmp(const x64* op1, const x64* op2);
    void EmitCmp(const x64* op1, unsigned long);
    void EmitTest(const x64* op1, const x64* op2);
    void EmitTest(const x64* op1, unsigned long);
    void EmitSet(const std::string& cond, const x64* dest);

private:
    char GetIntTag(const x64* op) const;
    std::string GetFltTag(const x64* op) const;

    std::string filename_{};
    FILE* file_{};
};

#endif // _EMIT_ASM_H_
