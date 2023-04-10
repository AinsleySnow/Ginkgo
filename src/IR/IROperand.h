#ifndef _IROPERAND_H_
#define _IROPERAND_H_

#include <memory>
#include <string>
#include <vector>
#include "IR/IRType.h"
#include "utils/Pool.h"


class IROperand
{
public:
    IROperand(const IRType* t) : type_(t) {}
    virtual ~IROperand() {}
    virtual std::string ToString() const = 0;

    virtual bool IsIntConst() const { return false; }
    virtual bool IsFloatConst() const { return false; }
    virtual bool IsConstant() const { return false; }
    virtual bool IsRegister() const { return false; }

    auto Type() const { return type_; }
    auto& Type() { return type_; }

protected:
    const IRType* type_{};
};


class Constant : public IROperand
{
public:
    Constant(const IRType* t) : IROperand(t) {}
    virtual bool IsZero() const = 0;
};

class IntConst : public Constant
{
public:
    static IntConst* CreateIntConst(Pool<IROperand>*, unsigned long);
    static IntConst* CreateIntConst(Pool<IROperand>*, unsigned long, const IntType*);
    IntConst(unsigned long ul, const IntType* t) :
        num_(ul), Constant(t) {}

    std::string ToString() const override;
    unsigned long Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }
    bool IsConstant() const override { return true; }
    bool IsIntConst() const override { return true; }

private:
    unsigned long num_{};
};

class FloatConst : public Constant
{
public:
    static FloatConst* CreateFloatConst(Pool<IROperand>*, double);
    static FloatConst* CreateFloatConst(Pool<IROperand>*, double, const FloatType*);
    FloatConst(double d, const FloatType* t) :
        num_(d), Constant(t) {}

    std::string ToString() const override;
    double Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }
    bool IsConstant() const override { return true; }
    bool IsFloatConst() const override { return true; }

private:
    double num_{};
};


class Register : public IROperand
{
public:
    static Register* CreateRegister(Pool<IROperand>*, const std::string&, const IRType*);
    Register(const std::string& n, const IRType* t) :
        name_(n), IROperand(t) {}

    std::string ToString() const override;
    std::string Name() const { return name_; }

private:
    std::string name_{};
};


enum class RegTag
{
    none,

    rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp,
    eax, ebx, ecx, edx, esi, edi, ebp, esp,
    ax, bx, cx, dx, si, di, bp, sp,
    al, bl, cl, dl, sil, dil, bpl, spl,

    r8, r9, r10, r11, r12, r13, r14, r15,
    r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d,
    r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w,
    r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b,

    xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7,
    xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15
};


class x64Reg : public IROperand
{
public:
    static x64Reg* CreateX64Reg(Pool<IROperand>*, RegTag = RegTag::none);
    x64Reg() : reg_(RegTag::none), IROperand(nullptr) {}
    x64Reg(RegTag r) : reg_(r), IROperand(nullptr) {}

    bool operator==(x64Reg reg) const { return reg_ == reg.reg_; }
    bool operator!=(x64Reg reg) const { return reg_ != reg.reg_; }
    bool operator==(RegTag tag) const { return reg_ == tag; }
    bool operator!=(RegTag tag) const { return reg_ != tag; }

    std::string ToString() const override;

private:
    RegTag reg_;
};


class x64Mem : public IROperand
{
public:
    static x64Mem* CreateX64Mem(Pool<IROperand>*, size_t = 0,
        const x64Reg* = nullptr, const x64Reg* = nullptr, size_t = 0);

    x64Mem(size_t o, const x64Reg* b, const x64Reg* i, size_t s) :
        offset_(o), base_(b), index_(i), scale_(s), IROperand(nullptr) {}

    std::string ToString() const override;

private:
    size_t offset_{};
    const x64Reg* base_{};
    const x64Reg* index_{};
    size_t scale_{};
};

#endif // _IROPERAND_H_
