#ifndef _PASS_H_
#define _PASS_H_

#include "IR/Value.h"


class Pass
{
public:
    Pass(Module* m) : module_(m) {}
    ~Pass() {}

    virtual void Execute() = 0;

    auto& CurModule() { return module_; }
    auto CurModule() const { return module_; }

private:
    Module* module_{};
};


class ModulePass : public Pass
{
public:
    ModulePass(Module* m) : Pass(m) {}
    void Execute() override { ExecuteOnModule(CurModule()); }
    virtual void ExecuteOnModule(Module*) = 0;
};


class FunctionPass : public Pass
{
public:
    FunctionPass(Module* m) : Pass(m) {}

    void Execute() override
    {
        for (auto pval : *CurModule())
        {
            if (auto func = pval->As<Function>(); func)
            {
                EnterFunction(func);
                ExecuteOnFunction(func);
            }
        }
    }

    virtual void ExecuteOnFunction(Function*) = 0;
    virtual void EnterFunction(Function*) = 0;
};

#endif // _PASS_H_
