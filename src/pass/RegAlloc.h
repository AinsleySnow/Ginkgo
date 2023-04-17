#ifndef _REG_ALLOC_H_
#define _REG_ALLOC_H_

#include "visitir/IRVisitor.h"
#include <unordered_map>
#include <unordered_set>

class BasicBlock;
class Register;
class x64;


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


// the reason that I use a template here is that, what if someone
// wants to implement a register allocator for another architecture?
// (maybe actually no one will, though T_T)
// Well, if you, sitting in front of the screen, really want to do that,
// then create a new file named x64Alloc.h, dump all x64-related code
// in this file into it, and write your allocator base class in a new file,
// say, riscvAlloc.h.
template <typename REGENUM, typename ARCHINFO>
class RegAlloc : protected IRVisitor
{
protected:
    void Mark(REGENUM r)         { inuse_.insert(r); used_.insert(r); }
    void Unmark(REGENUM r)       { inuse_.erase(r); }
    bool InUse(REGENUM r) const  { return inuse_.find(r) != inuse_.end(); }

    ARCHINFO& InfoAt(const Function* f) { return infomap_[f]; }
    const ARCHINFO& InfoAt(const Function* f) const { return infomap_.at(f); }

private:
    std::unordered_set<REGENUM> inuse_{};
    std::unordered_set<REGENUM> used_{};
    std::unordered_map<const Function*, ARCHINFO> infomap_{};
};


class x64Alloc : protected RegAlloc<x64Phys, x64Stack>
{
protected:
    inline size_t MakeAlign(size_t base, size_t align) const
    { return (base + 16) % align == 0 ? base : (base + 16) + align - (base + 16) % align; }

    const x64* MapConstAndGlobalVar(BasicBlock* bb, const IROperand* op)
    {
        if (op->Is<Constant>())
            return x64Imm::CreateX64Imm(bb, op->As<Constant>());

        auto reg = op->As<Register>();
        if (reg->Name()[0] == '@')
            return x64Mem::CreateX64Mem(bb, reg->Name().substr(1));

        return nullptr;
    }
};

#endif // _REG_ALLOC_H_
