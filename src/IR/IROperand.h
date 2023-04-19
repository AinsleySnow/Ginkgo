#ifndef _IROPERAND_H_
#define _IROPERAND_H_

#include <memory>
#include <string>
#include <vector>
#include "IR/IRType.h"
#include "utils/DynCast.h"
#include "utils/Pool.h"


class IROperand
{
protected:
    enum class OpId { op, _int, _float, reg, x64reg, x64mem, x64imm };
    static bool ClassOf(const IROperand const*) { return true; }
    OpId id_ = OpId::op;

public:
    IROperand(OpId i, const IRType* t) : id_(i), type_(t) {}
    virtual ~IROperand() {}
    virtual std::string ToString() const = 0;

    OpId ID() const { return id_; }

    ENABLE_IS;
    ENABLE_AS;

    auto Type() const { return type_; }
    auto& Type() { return type_; }

protected:
    const IRType* type_{};
};


class Constant : public IROperand
{
protected:
    static bool ClassOf(const Constant const*) { return true; }
    static bool ClassOf(const IROperand const* op)
    { return op->ID() == OpId::_int || op->ID() == OpId::_float; }

public:
    Constant(OpId i, const IRType* t) : IROperand(i, t) {}
    virtual bool IsZero() const = 0;
};

class IntConst : public Constant
{
protected:
    static bool ClassOf(const IntConst const*) { return true; }
    static bool ClassOf(const Constant const* c) { return c->ID() == OpId::_int; }

public:
    static IntConst* CreateIntConst(Pool<IROperand>*, unsigned long);
    static IntConst* CreateIntConst(Pool<IROperand>*, unsigned long, const IntType*);
    IntConst(unsigned long ul, const IntType* t) :
        num_(ul), Constant(OpId::_int, t) {}

    std::string ToString() const override;
    unsigned long Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }

private:
    unsigned long num_{};
};

class FloatConst : public Constant
{
protected:
    static bool ClassOf(const FloatConst const*) { return true; }
    static bool ClassOf(const Constant const* c) { return c->ID() == OpId::_float; }

public:
    static FloatConst* CreateFloatConst(Pool<IROperand>*, double);
    static FloatConst* CreateFloatConst(Pool<IROperand>*, double, const FloatType*);
    FloatConst(double d, const FloatType* t) :
        num_(d), Constant(OpId::_float, t) {}

    std::string ToString() const override;
    double Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }

private:
    double num_{};
};


class Register : public IROperand
{
protected:
    static bool ClassOf(const Register const*) { return true; }
    static bool ClassOf(const IROperand const* op) { return op->ID() == OpId::reg; }

public:
    static Register* CreateRegister(Pool<IROperand>*, const std::string&, const IRType*);
    Register(const std::string& n, const IRType* t) :
        name_(n), IROperand(OpId::reg, t) {}

    std::string ToString() const override;
    std::string Name() const { return name_; }

private:
    std::string name_{};
};


enum class RegTag
{
    none, rip,

    rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp,
    eax, ebx, ecx, edx, esi, edi, ebp, esp,
     ax,  bx,  cx,  dx,  si,  di,  bp,  sp,
     al,  bl,  cl,  dl, sil, dil, bpl, spl,

     r8,  r9,  r10,  r11,  r12,  r13,  r14, r15,
    r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d,
    r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w,
    r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b,

    xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7,
    xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15
};


class x64 : public IROperand
{
protected:
    static bool ClassOf(const x64 const*) { return true; }
    static bool ClassOf(const IROperand const* i)
    { return i->ID() == OpId::x64reg || i->ID() == OpId::x64mem; }

public:
    x64(OpId i) : IROperand(i, nullptr) {}
};


class x64Reg : public x64
{
protected:
    static bool ClassOf(const x64Reg const*) { return true; }
    static bool ClassOf(const IROperand const* i) { return i->ID() == OpId::x64reg; }

public:
    static x64Reg* CreateX64Reg(Pool<IROperand>*, RegTag = RegTag::none);
    x64Reg() : x64(OpId::x64reg), reg_(RegTag::none) {}
    x64Reg(RegTag r) : x64(OpId::x64reg), reg_(r) {}

    bool operator==(x64Reg& reg) const { return reg_ == reg.reg_; }
    bool operator!=(x64Reg& reg) const { return reg_ != reg.reg_; }
    bool operator==(RegTag tag) const { return reg_ == tag; }
    bool operator!=(RegTag tag) const { return reg_ != tag; }
    bool PartOf(x64Reg&) const;
    bool PartOf(RegTag) const;

    std::string ToString() const override;

private:
    RegTag reg_;
};


class x64Mem : public x64
{
protected:
    static bool ClassOf(const x64Mem const*) { return true; }
    static bool ClassOf(const IROperand const* i) { return i->ID() == OpId::x64mem; }

public:
    static x64Mem* CreateX64Mem(Pool<IROperand>*, const std::string&);
    static x64Mem* CreateX64Mem(Pool<IROperand>*, long = 0,
        const x64Reg* = nullptr, const x64Reg* = nullptr, size_t = 0);

    x64Mem(const std::string& l) : x64(OpId::x64mem), label_(l) {}
    x64Mem(size_t o, const x64Reg* b, const x64Reg* i, size_t s) :
        x64(OpId::x64mem), offset_(o), base_(b), index_(i), scale_(s) {}

    std::string ToString() const override;

private:
    std::string label_{};

    long offset_{};
    const x64Reg* base_{};
    const x64Reg* index_{};
    size_t scale_{};
};


class x64Imm : public x64
{
protected:
    static bool ClassOf(const x64Mem const*) { return true; }
    static bool ClassOf(const IROperand const* i) { return i->ID() == OpId::x64imm; }

public:
    static x64Imm* CreateX64Imm(Pool<IROperand>*, const Constant*);
    x64Imm(const Constant* c) : x64(OpId::x64imm), val_(c) {}

    std::string ToString() const override;

private:
    const Constant* val_{};
};

#endif // _IROPERAND_H_
