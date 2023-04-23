#ifndef _X64_H_
#define _X64_H_

#include "visitir/RegAlloc.h"
#include "utils/DynCast.h"
#include <string>

class Constant;


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


class x64
{
protected:
    enum class x64Id { reg, mem, imm };
    static bool ClassOf(const x64 const*) { return true; }
    x64Id id_{};

public:
    x64(x64Id i) {}
    x64Id ID() const { return id_; }

    ENABLE_IS;
    ENABLE_AS;

    virtual std::string ToString() const = 0;
};


class x64Reg : public x64
{
protected:
    static bool ClassOf(const x64Reg const*) { return true; }
    static bool ClassOf(const x64 const* i) { return i->ID() == x64Id::reg; }

public:
    x64Reg() : x64(x64Id::reg), reg_(RegTag::none) {}
    x64Reg(RegTag r) : x64(x64Id::reg), reg_(r) {}

    bool operator==(x64Reg& reg) const { return reg_ == reg.reg_; }
    bool operator!=(x64Reg& reg) const { return reg_ != reg.reg_; }
    bool operator==(RegTag tag) const { return reg_ == tag; }
    bool operator!=(RegTag tag) const { return reg_ != tag; }
    bool PartOf(x64Reg&) const;
    bool PartOf(RegTag) const;

    std::string ToString() const override;

    auto& Suffix() { return suffix_; }
    auto Suffix() const { return suffix_; }

private:
    RegTag reg_;
    std::string suffix_{};
};


class x64Mem : public x64
{
protected:
    static bool ClassOf(const x64Mem const*) { return true; }
    static bool ClassOf(const x64 const* i) { return i->ID() == x64Id::mem; }

public:
    x64Mem(const std::string& l) : x64(x64Id::mem), label_(l) {}
    x64Mem(size_t o, RegTag b, RegTag i, size_t s) :
        x64(x64Id::mem), offset_(o), base_(b), index_(i), scale_(s) {}

    std::string ToString() const override;

private:
    std::string label_{};

    long offset_{};
    RegTag base_{};
    RegTag index_{};
    size_t scale_{};
};


class x64Imm : public x64
{
protected:
    static bool ClassOf(const x64Mem const*) { return true; }
    static bool ClassOf(const x64 const* i) { return i->ID() == x64Id::imm; }

public:
    x64Imm(const Constant* c) : x64(x64Id::imm), val_(c) {}
    std::string ToString() const override;

private:
    const Constant* val_{};
};


enum class x64Phys
{
    rax, rbx, rcx, rdx,
    rsi, rdi, rbp, rsp,
    r8, r9, r10, r11,
    r12, r13, r14, r15,

    xmm0, xmm1, xmm2, xmm3,
    xmm4, xmm5, xmm6, xmm7,
    xmm8, xmm9, xmm10, xmm11,
    xmm12, xmm13, xmm14, xmm15,
};

//      high address
// +-------------------+
// |    above rsp      |
// +-------rsp---------+
// |    below rsp      | <- red zone, 128 bytes
// +-------------------+
//      low address

struct x64Stack
{
    // the offset of rsp itself.
    long rspoffset_{};
    // for leaf functions; how many bytes are used below rsp?
    size_t belowrsp_{};
    // how many bytes we'll allocate on the stack.
    size_t allocated_{};
    // is current function a leaf function?
    bool leaf_{ true };
};


class x64Alloc : protected RegAlloc<x64Phys, x64Stack>
{
public:
    virtual void Clear() = 0;

    void DoAlloc(Function* func) { VisitFunction(func); }
    const x64* GetIROpMap(const IROperand* op) const;

protected:
    inline size_t MakeAlign(size_t base, size_t align) const;
    bool MapConstAndGlobalVar(const IROperand* op);
    void MapRegister(const IROperand*, std::unique_ptr<x64>);
    void Clearx64() { irmap_.clear(); }

private:
    std::unordered_map<const IROperand*, std::unique_ptr<x64>> irmap_{};
};

#endif // _X64_H_
