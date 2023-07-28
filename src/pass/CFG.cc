#include "pass/CFG.h"
#include "IR/Instr.h"
#include "IR/Value.h"


void CFG::VisitFunction(Function* func)
{
    FlowGraph fg{};

    for (auto bb : *func)
        fg.AddVertex(bb);
    // Stands for exit
    fg.AddVertex(nullptr);

    for (auto bb : *func)
        VisitBasicBlock(func, bb);
    flow_[func] = std::move(fg);
}

void CFG::VisitBasicBlock(Function* func, BasicBlock* bb)
{
    for (auto i : *bb)
    {
        switch (i->id_)
        {
        case Instr::InstrId::br:
            VisitBrInstr(func, bb, i->As<BrInstr>());
            break;
        case Instr::InstrId::call:
            VisitCallInstr(func, bb, i->As<CallInstr>());
            break;
        case Instr::InstrId::swtch:
            VisitSwitchInstr(func, bb, i->As<SwitchInstr>());
            break;
        case Instr::InstrId::ret:
            VisitRetInstr(func, bb, i->As<RetInstr>());
            break;
        }
    }
}

void CFG::VisitBrInstr(Function* func, BasicBlock* bb, BrInstr* br)
{
    if (br->Cond())
    {
        flow_[func].AddValueEdge(
            bb, br->GetTrueBlk(), { br->Cond(), true });
        flow_[func].AddValueEdge(
            bb, br->GetFalseBlk(), { br->Cond(), false });
        preds_[br->GetTrueBlk()].insert(bb);
        preds_[br->GetFalseBlk()].insert(bb);
    }
    else
    {
        flow_[func].AddValueEdge(
            bb, br->GetTrueBlk(), { nullptr, true });
        preds_[br->GetTrueBlk()].insert(bb);
    }
}

void CFG::VisitCallInstr(Function* func, BasicBlock* bb, CallInstr* c)
{
    if (c->FuncName().empty())
        return;
    calling_.AddValueEdge(func,
        CurModule()->GetFunction(c->FuncName()), { bb, c });
}

void CFG::VisitSwitchInstr(Function* func, BasicBlock* bb, SwitchInstr* s)
{
    for (auto [i, to] : s->GetValueBlkPairs())
    {
        flow_[func].AddValueEdge(bb, to, { i, true });
        preds_[to].insert(bb);
    }

    flow_[func].AddValueEdge(
        bb, s->GetDefault(), { s->GetIdent(), false });
    preds_[s->GetDefault()].insert(bb);
}

void CFG::VisitRetInstr(Function* func, BasicBlock* bb, RetInstr* r)
{
    flow_[func].AddValueEdge(bb, nullptr, { nullptr, true });
}


void CFG::ExecuteOnModule(Module* mod)
{
    for (auto pval : *mod)
        if (auto f = pval->As<Function>(); f)
            calling_.AddVertex(f);

    for (auto pval : *mod)
        if (auto f = pval->As<Function>(); f)
            VisitFunction(f);
}
