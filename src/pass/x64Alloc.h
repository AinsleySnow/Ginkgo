#ifndef _X64_ALLOC_H_
#define _X64_ALLOC_H_

#include "pass/RegAlloc.h"
#include <memory>
#include <unordered_map>

class Function;
class IROperand;
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


class x64Alloc : public FunctionPass, protected IRVisitor, public RegAlloc<x64Phys, x64Stack>
{
public:
    x64Alloc(Module* m) : FunctionPass(m) {}

    void Execute() override;
    void EnterFunction(Function* func) override { curfunc_ = func; }
    void ExecuteOnFunction(Function* func) override { VisitFunction(func); }

    const x64* GetIROpMap(const IROperand* op) const;

protected:
    inline size_t MakeAlign(size_t base, size_t align) const;
    bool MapConstAndGlobalVar(const IROperand* op);
    void MapRegister(const IROperand*, std::unique_ptr<x64>);

private:
    Function* curfunc_{};
    std::unordered_map<Function*,
        std::unordered_map<
            const IROperand*, std::unique_ptr<x64>>> irmap_{};
};


#endif // _X64_ALLOC_H_
