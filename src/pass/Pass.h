#ifndef _PASS_H_
#define _PASS_H_

#include "IR/Value.h"
#include "utils/DynCast.h"


class Pass
{
public:
    enum class PassId { none, _module, function };

    static bool ClassOf(const Pass*) { return true; }
    PassId id_ = PassId::none;

    Pass(PassId p, Module* m) : id_(p), module_(m) {}
    virtual ~Pass() {}

    ENABLE_IS;
    ENABLE_AS;

    auto& CurModule() { return module_; }
    auto CurModule() const { return module_; }

private:
    Module* module_{};
};


class ModulePass : public Pass
{
public:
    static bool ClassOf(const ModulePass*) { return true; }
    static bool ClassOf(const Pass* p) { return p->id_ == PassId::_module; }

    ModulePass(Module* m) : Pass(PassId::_module, m) {}
    virtual void ExecuteOnModule() = 0;
};


class FunctionPass : public Pass
{
public:
    static bool ClassOf(const FunctionPass*) { return true; }
    static bool ClassOf(const Pass* p) { return p->id_ == PassId::function; }

    FunctionPass(Module* m) : Pass(PassId::function, m) {}
    virtual void ExecuteOnFunction(Function*) = 0;
    virtual void ExitFunction() = 0;
};

#endif // _PASS_H_
