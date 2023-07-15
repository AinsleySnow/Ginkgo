#ifndef _X64_H_
#define _X64_H_

#include "utils/DynCast.h"
#include <cstring>
#include <string>

class Constant;


template <typename T>
std::pair<unsigned long, unsigned long> FloatRepr(T fp)
{
    unsigned long repr[2] = { 0, 0 };
    std::memcpy(reinterpret_cast<char*>(&repr),
        reinterpret_cast<char*>(&fp), sizeof(T));
    return std::make_pair(repr[0], repr[1]);
}


enum class RegTag
{
    none, rip,

    rax, rbx, rcx, rdx,
    rsi, rdi, rbp, rsp,
    r8,  r9,  r10, r11,
    r12, r13, r14, r15,

    xmm0, xmm1, xmm2, xmm3,
    xmm4, xmm5, xmm6, xmm7,
    xmm8, xmm9, xmm10, xmm11,
    xmm12, xmm13, xmm14, xmm15
};


class x64
{
public:
    enum class x64Id { reg, mem, imm };
    static bool ClassOf(const x64* const) { return true; }
    x64Id id_{};

    x64(x64Id i, size_t s) : id_(i), size_(s) {}

    ENABLE_IS;
    ENABLE_AS;

    bool operator==(const x64& x) const;
    bool operator!=(const x64& x) const { return !(*this == x); }
    bool operator==(RegTag t) const;
    bool operator!=(RegTag t) const { return !(*this == t); }

    virtual std::string ToString() const = 0;

    auto& Size() { return size_; }
    auto Size() const { return size_; }

private:
    size_t size_{};
};


class x64Reg : public x64
{
public:
    static bool ClassOf(const x64Reg* const) { return true; }
    static bool ClassOf(const x64* const i) { return i->id_ == x64Id::reg; }

    x64Reg(size_t s) : x64(x64Id::reg, s), reg_(RegTag::none) {}
    x64Reg(RegTag r) : x64(x64Id::reg, 8), reg_(r) {}
    x64Reg(RegTag r, size_t s) : x64(x64Id::reg, s), reg_(r) {}

    bool operator==(const x64Reg& reg) const { return reg_ == reg.reg_ && Size() == reg.Size(); }
    bool operator!=(const x64Reg& reg) const { return !(*this == reg); }
    bool operator==(RegTag tag) const { return reg_ == tag; }
    bool operator!=(RegTag tag) const { return reg_ != tag; }

    std::string ToString() const override;
    RegTag Tag() const { return reg_; }

private:
    RegTag reg_;
};


class x64Mem : public x64
{
public:
    static bool ClassOf(const x64Mem* const) { return true; }
    static bool ClassOf(const x64* const i) { return i->id_ == x64Id::mem; }

    x64Mem(size_t sz, const std::string& l) : x64(x64Id::mem, sz), label_(l) {}
    x64Mem(size_t sz, long o, const x64Reg& b, const x64Reg& i, size_t s) :
        x64(x64Id::mem, sz), offset_(o), base_(b), index_(i), scale_(s) {}

    std::string ToString() const override;

    bool operator==(const x64Mem& mem) const;
    bool operator!=(const x64Mem& mem) const { return !(*this == mem); }

    bool GlobalLoc() const { return !label_.empty(); }
    bool& LoadTwice() { return loadtwice_; }
    bool LoadTwice() const { return loadtwice_; }

    auto Offset() const { return offset_; }
    auto& Base() const { return base_; }
    auto& Index() const { return index_; }
    auto Scale() const { return scale_; }

private:
    // The register allocator can map a pointer to either the stack
    // or a register. If a pointer is mapped to the stack, there
    // comes two situations: one, only by lea instruction can we get
    // the value of the pointer; two, the value of the pointer can be
    // loaded by mov instruction like common variable. The loadtwice_
    // field distinguish the two situations - false for the first
    // situation, and true for the second one.
    bool loadtwice_{};

    std::string label_{};

    long offset_{};
    x64Reg base_{ 0 };
    x64Reg index_{ 0 };
    size_t scale_{};
};


class x64Imm : public x64
{
public:
    static bool ClassOf(const x64Mem* const) { return true; }
    static bool ClassOf(const x64* const i) { return i->id_ == x64Id::imm; }

    x64Imm(const Constant* c);

    std::pair<unsigned long, unsigned long> GetRepr() const;
    std::string ToString() const override;

    bool operator==(const x64Imm& imm) const;
    bool operator!=(const x64Imm& imm) const { return !(*this == imm); }

private:
    const Constant* val_{};
};


#endif // _X64_H_
