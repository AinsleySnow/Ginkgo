#include "pass/CallingGraph.h"
#include "IR/Instr.h"
#include "IR/Value.h"
#include <fmt/format.h>


void CallingGraph::VisitFunction(Function* func)
{
    if (func->Empty())
        return;

    for (auto bb : *func)
        for (auto i : *bb)
            if (auto c = i->As<CallInstr>(); c)
                VisitCallInstr(c);
}

void CallingGraph::VisitCallInstr(Function* f, CallInstr* c)
{
    if (c->FuncName().empty())
        return;
    // return if we're calling a builtin function
    if (c->FuncName() == "@__Ginkgo_assert")
        return;

    calling_.AddValueEdge(func,
        CurModule()->GetFunction(c->FuncName()), { bb, c });
}


std::string CallingGraph::PrintSummary() const
{
    std::string summary{ "Pass CallingGraph:\n" };
    for (auto& [v, adj] : calling_)
        for (auto& [to, info] : adj)
            summary += fmt::format(
                "{} -> {} in block {}\n", v->Name(), info.first->Name());
    return std::move(summary);
}


void CallingGraph::ExecuteOnModule()
{
    for (auto v : *CurModule())
        if (auto f = v->As<Function>(); f)
            VisitFunction(f);
}
