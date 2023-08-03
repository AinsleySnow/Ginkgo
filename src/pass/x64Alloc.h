#ifndef _X64_ALLOC_H_
#define _X64_ALLOC_H_

#include "pass/Pass.h"
#include "pass/RegAlloc.h"
#include <memory>
#include <set>
#include <unordered_map>

class Function;
class IROperand;
class x64;


enum class x64Phys
{
    rax, rbx, rcx, rdx,
    rsi, rdi, rbp, rsp,
    r8,  r9,  r10, r11,
    r12, r13, r14, r15,

    xmm0,  xmm1,  xmm2,  xmm3,
    xmm4,  xmm5,  xmm6,  xmm7,
    xmm8,  xmm9,  xmm10, xmm11,
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


// Tips on writing new register allocators:
// 
// An assumption is held by the code generator that RAX, R11 and XMM0
// are ALWAYS able to be used as temporary registers, even if all the
// physics registers are allocated. So the allocator must be careful
// to guarantee that writing to these registers arbitraily will not
// break up the program. Reserving these registers is design compromise
// since we do not have lower IR here.
// 
// Another assumption made by the code generator is that, RDI, RSI
// and other registers used to pass parameters can be safely overwritten
// when there is a function call.
// 
// After finishing mapping registers to the stack, it is required for the
// register allocators that to align the stack size by 16. The code generator
// always treats the stack as aligned by 16 in the beginning of the function.
// 
// The allocator can simply ignore the platform requirement of instructions
// operand type, and the code generator will handle all these annoying stuff.

class x64Alloc : public FunctionPass, protected IRVisitor, public RegAlloc<x64Phys, x64Stack>
{
public:
    using RegSet = std::set<x64Phys>;
    using RegX64Map = std::unordered_map<const IROperand*, std::unique_ptr<x64>>;

    x64Alloc(Module* m) : FunctionPass(m) {}

    void EnterFunction(Function* func) { curfunc_ = func; }
    void ExitFunction(Function* func) override
    {
        ArchInfo() = x64Stack();
        Clear();
    }
    void ExecuteOnFunction(Function* func) override
    {
        curfunc_ = func;
        VisitFunction(func);
        regmap_.emplace(func, std::move(UsedRegs()));
        infomap_.emplace(func, std::move(ArchInfo()));
    }

    const x64* GetIROpMap(const IROperand* op) const;
    long RspOffset() const { return infomap_.at(curfunc_).rspoffset_; }

    RegSet UsedCallerSaved() const;
    RegSet UsedCalleeSaved() const;
    RegSet UsedIntReg() const;
    RegSet UsedVecReg() const;
    RegSet NotUsedIntReg() const;
    RegSet NotUsedVecReg() const;

protected:
    inline size_t MakeAlign(size_t base, size_t align) const
    {
        return base % align == 0 ?
            base : base + align - base % align;
    }

    void LoadParam();
    bool MapConstAndGlobalVar(const IROperand* op);
    void MapRegister(const IROperand*, std::unique_ptr<x64>);
    void MarkLoadTwice(const IROperand*);

private:
    Function* curfunc_{};
    std::unordered_map<const Function*, x64Stack> infomap_{};
    std::unordered_map<const Function*, RegX64Map> irmap_{};
    std::unordered_map<const Function*, RegSet> regmap_{};
};


#endif // _X64_ALLOC_H_
