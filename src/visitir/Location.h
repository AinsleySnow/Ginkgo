#ifndef _LOCATION_H_
#define _LOCATION_H_

#include <string>


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


class RegLoc
{
public:
    RegLoc() : reg_(RegTag::none) {}
    RegLoc(RegTag r) : reg_(r) {}

    bool operator==(RegLoc reg) const { return reg_ == reg.reg_; }
    bool operator!=(RegLoc reg) const { return reg_ != reg.reg_; }
    bool operator==(RegTag tag) const { return reg_ == tag; }
    bool operator!=(RegTag tag) const { return reg_ != tag; }

    std::string ToString() const;

private:
    RegTag reg_;
};


class MemLoc
{
public:
    MemLoc(size_t o, RegLoc b, RegLoc i, size_t s) :
        offset_(o), base_(b), index_(i), scale_(s) {}

    std::string ToString() const;

private:
    size_t offset_{};
    RegLoc base_{};
    RegLoc index_{};
    size_t scale_{};
};

#endif // _LOCATION_H_
