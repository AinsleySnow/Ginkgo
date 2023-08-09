#include "pass/FlowGraph.h"
#include "IR/Instr.h"
#include "IR/Value.h"
#include <fmt/format.h>


void FlowGraph::VisitBasicBlock(BasicBlock* bb)
{
    for (auto i : *bb)
    {
        switch (i->id_)
        {
        case Instr::InstrId::br:
            VisitBrInstr(bb, i->As<BrInstr>());
            break;
        case Instr::InstrId::swtch:
            VisitSwitchInstr(bb, i->As<SwitchInstr>());
            break;
        case Instr::InstrId::ret:
            VisitRetInstr(bb, i->As<RetInstr>());
            break;
        }
    }
}

void FlowGraph::VisitBrInstr(BasicBlock* bb, BrInstr* br)
{
    if (br->Cond())
    {
        flow_.AddValueEdge(
            bb, br->GetTrueBlk(), { br->Cond(), true });
        flow_.AddValueEdge(
            bb, br->GetFalseBlk(), { br->Cond(), false });
        preds_.emplace(br->GetTrueBlk(), bb);
        preds_.emplace(br->GetFalseBlk(), bb);
    }
    else
    {
        flow_.AddValueEdge(
            bb, br->GetTrueBlk(), { nullptr, true });
        preds_.emplace(br->GetTrueBlk(), bb);
    }
}

void FlowGraph::VisitSwitchInstr(BasicBlock* bb, SwitchInstr* s)
{
    for (auto [i, to] : s->GetValueBlkPairs())
    {
        flow_.AddValueEdge(bb, to, { i, true });
        preds_.emplace(to, bb);
    }

    flow_.AddValueEdge(
        bb, s->GetDefault(), { s->GetIdent(), false });
    preds_.emplace(s->GetDefault(), bb);
}

void FlowGraph::VisitRetInstr(BasicBlock* bb, RetInstr* r)
{
    flow_.AddValueEdge(bb, nullptr, { nullptr, true });
}


std::string FlowGraph::PrintSummary() const
{
    std::string summary{ fmt::format("Pass FlowGraph in function {}:\n", CurFunc()->Name()) };
    for (auto [v, adj] : flow_)
    {
        for (auto [to, cond] : *adj)
        {
            if (to == nullptr)
                summary += fmt::format("{} -> exit\n", (**v)->Name());
            else
            {
                summary += fmt::format("{} -> {}", (**v)->Name(), to->Name());
                if (!cond.first)
                    summary += '\n';
                else if (cond.second)
                    summary += fmt::format(", {}\n", cond.first->ToString());
                else
                    summary += ", otherwise\n";
            }
        }
    }
    return std::move(summary);
}


void FlowGraph::ExecuteOnFunction(Function* func)
{
    CurFunc() = func;
    for (auto bb : *func)
        flow_.AddVertex(bb);
    // Stands for exit
    flow_.AddVertex(nullptr);
    for (auto bb : *func)
        VisitBasicBlock(bb);
}
