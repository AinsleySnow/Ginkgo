#include "pass/x64Alloc.h"
#include "IR/IROperand.h"
#include "IR/Value.h"
#include "visitir/x64.h"


bool x64Alloc::MapConstAndGlobalVar(const IROperand* op)
{
    if (op->Is<Constant>())
    {
        irmap_[curfunc_][op] = std::make_unique<x64Imm>(op->As<Constant>());
        return true;
    }
    auto reg = op->As<Register>();
    if (reg->Name()[0] == '@')
    {
        irmap_[curfunc_][op] = std::make_unique<x64Mem>(reg->Name().substr(1));
        return true;
    }
    return false;
}

void x64Alloc::MapRegister(const IROperand* op, std::unique_ptr<x64> reg)
{
    irmap_[curfunc_][op] = std::move(reg);
}


void x64Alloc::Execute()
{
    for (auto sym : *CurModule())
    {
        if (!sym->Is<Function>()) continue;
        auto func = sym->As<Function>();
        EnterFunction(func);
        ExecuteOnFunction(func);
    }
}

const x64* x64Alloc::GetIROpMap(const IROperand* op) const
{
    auto it = irmap_.at(curfunc_).find(op);
    if (it == irmap_.at(curfunc_).end()) return nullptr;
    return it->second.get();
}
