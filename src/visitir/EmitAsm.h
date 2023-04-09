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
    void EmitPseudoInstr(
        const std::string&, std::initializer_list<std::string>);

    void EmitLeaq(const std::string&, const std::string&);
    void EmitUnary(char, const std::string&, const std::string&);
    void EmitBinary(char, const std::string&, const std::string&, const std::string&);

    void EmitVarithm(const std::string&, const std::string&, const std::string&,
        const std::string&, const std::string&);
    void EmitVcvtt(const std::string&, const std::string&,
        const std::string&, const std::string&);
    void EmitVcvt(const std::string&, const std::string&, const std::string&,
        const std::string&, const std::string&);
    void EmitUcom(const std::string&, const std::string&, const std::string&);

    void EmitMov(char, const std::string&, const std::string&);
    void EmitMovz(char, char, const std::string&, const std::string&);
    void EmitMovs(char, char, const std::string&, const std::string&);
    void EmitVmov(const std::string&, const std::string&, const std::string&);

    void EmitPop(char, const std::string&);
    void EmitPush(char, const std::string&);

    void EmitCall(const std::string&);
    void EmitRet();

    void EmitJmp(const std::string&, const std::string&);
    void EmitCMov(const std::string&, const std::string&, const std::string&);

    void EmitCmp(char, const std::string&, const std::string&);
    void EmitTest(char, const std::string&, const std::string&);
    void EmitSet(const std::string&, const std::string&);


private:
    std::string filename_{};
    FILE* file_{};
};

#endif // _EMIT_ASM_H_
