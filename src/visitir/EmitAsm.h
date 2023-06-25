#ifndef _EMIT_ASM_H_
#define _EMIT_ASM_H_

#include <cstdio>
#include <fstream>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

class BasicBlock;
class x64;
class x64Reg;

enum class RegTag;


class EmitAsm
{
public:
    EmitAsm(const std::string& fn) : filename_(fn) { file_.open(fn); }
    ~EmitAsm() { file_.close(); }

    std::string AsmName() const { return filename_; }

    auto CurBlock() const { return curblk_; }
    void EnterBlock(const BasicBlock*);
    void SwitchBlock(const BasicBlock*);

    void Write2Mem() { write2file_ = false; }
    void Dump2File();

    void EmitLabel(const std::string&);
    void EmitPseudoInstr(const std::string&);
    void EmitPseudoInstr(
        const std::string&, std::initializer_list<std::string>);

    void EmitCxtx(size_t);
    void EmitLeaq(const x64* addr, const x64* dest);
    void EmitLeaq(const x64* addr, RegTag dest);
    void EmitUnary(const std::string& instr, const x64* op);
    void EmitBinary(const std::string& instr, unsigned long imm, const x64* op);
    void EmitBinary(const std::string& instr, const x64* op1, const x64* op2);

    void EmitVarithm(const std::string& instr, const x64* op1,
        const x64* op2, const x64* dest);
    void EmitVarithm(const std::string& instr, RegTag op1, RegTag op2, RegTag op3);

    void EmitVcvtt(const x64* src, const x64* dest);
    void EmitVcvtt(const x64* op1, RegTag op2);
    void EmitVcvtt(RegTag op1, const x64* op2);

    void EmitVcvt(const x64* op1, const x64* dest);
    void EmitVcvt(const x64* op1, RegTag op2);
    void EmitVcvt(RegTag op1, const x64* op2);

    void EmitUcom(const x64* op1, const x64* op2);
    void EmitUcom(RegTag op1, const x64* op2);

    void EmitMov(const x64* src, const x64* dest);
    void EmitMov(RegTag, const x64* dest);
    void EmitMov(const x64* src, RegTag);
    void EmitMov(RegTag, long offset);
    void EmitMovz(const x64* src, const x64* dest);
    void EmitMovz(size_t from, size_t to, const x64* op);
    void EmitMovs(const x64* src, const x64* dest);
    void EmitMovs(size_t from, size_t to, const x64* op);

    void EmitVmov(const x64* src, const x64* dest);
    void EmitVmov(const x64* src, RegTag);
    void EmitVmov(RegTag, const x64* dest);
    void EmitVmov(RegTag, long offset);
    void EmitVmovap(const x64Reg* src, const x64Reg* dest);
    void EmitVmovap(const x64Reg* src, RegTag dest);
    void EmitVmovap(RegTag, const x64Reg*);

    void EmitPop(const x64* dest);
    void EmitPop(RegTag, size_t);
    void EmitPush(const x64* dest);
    void EmitPush(RegTag, size_t);

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
    void Output(const std::string&);

    mutable int labelindex_{};

    std::unordered_map<
        const BasicBlock*, std::vector<std::string>> blks_{};
    std::vector<const BasicBlock*> blkindexes_{};
    const BasicBlock* curblk_{};

    std::string filename_{};
    std::ofstream file_{};
    bool write2file_{ true };
};

#endif // _EMIT_ASM_H_
