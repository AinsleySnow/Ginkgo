#include "pass/Liveness.h"
#include "utils/Graph.h"
#include <unordered_map>
#include <unordered_set>


void Liveness::PartialLiveness(const FlowGraph::GraphType& fg, const BasicBlock* bb)
{
    onpath_.insert(bb);

    for (auto v : fg[bb])
    {
        auto to = v.to_;
        // Is the edge a back edge or a visited vertex?
        if (OnPath(to) || Visited(to))
            continue;
        // Re-entry edge?
        if (loops_->IsReenrty(bb, to))
            PartialLiveness(fg, FindOLE(bb, to));
        else
            PartialLiveness(fg, to);
    }

    std::unordered_set<const IROperand*> live{};
    // live <- PhiUses(bb)
    if (duinfo_->HasPhiUse(bb))
        for (auto op : duinfo_->GetPhiUse(bb))
            live.insert(op);
    // live <- (U(s in succ(bb)) (LiveIn(s) \ PhiDefs(s))) U live
    for (auto v : fg[bb])
    {
        if (OnPath(v.to_))
            continue;
        auto to = v.to_;
        if (loops_->IsReenrty(bb, to))
            to = FindOLE(bb, to);
        live.merge(livein_[to]);
        if (duinfo_->HasPhiDef(to))
            live.erase(duinfo_->GetPhiDef(to));
    }
    // copy to liveout
    liveout_[bb] = live;

    // LiveIn(bb) = PhiDefs(bb) U UpwardExposed(bb) U (LiveOut(bb) \ Defs(bb))
    // UpwardExposed(bb) : variables used in bb without any preceding definition in bb
    // remove variables defined in bb
    if (duinfo_->HasDef(bb))
        for (auto op : duinfo_->GetDef(bb))
            live.erase(op);
    // add variables used in bb to live
    if (duinfo_->HasUse(bb))
        for (auto op : duinfo_->GetUse(bb))
            live.insert(op);
    // add variable defined by phi instruction
    if (duinfo_->HasPhiDef(bb))
        live.insert(duinfo_->GetPhiDef(bb));
    livein_[bb] = std::move(live);

    onpath_.erase(bb);
    visited_.insert(bb);
}

void Liveness::PropagateHeader(const Function* func)
{
    // If a variable is live-in at the header of a loop,
    // then it is live at all nodes inside the loop.
    std::unordered_map<const BasicBlock*, std::unordered_set<const IROperand*>> liveloop{};
    for (auto bb : *func)
    {
        auto header = loops_->GetHeader(bb);
        while (header)
        {
            if (auto set = liveloop.find(header); set != liveloop.end())
            {
                livein_[bb].merge(set->second);
                liveout_[bb].merge(set->second);
            }
            else
            {
                auto temp = livein_[header];
                if (duinfo_->HasPhiDef(header))
                    temp.erase(duinfo_->GetPhiDef(header));
                livein_[bb].merge(temp);
                liveout_[bb].merge(temp);
                liveloop[header] = std::move(temp);
            }
            header = loops_->GetHeader(header);
        }
    }
}

const BasicBlock* Liveness::FindOLE(
    const BasicBlock* from, const BasicBlock* to) const
{
    std::unordered_set<const BasicBlock*> ancfrom{};
    auto header = loops_->GetHeader(from);
    while (header)
    {
        ancfrom.insert(header);
        header = loops_->GetHeader(header);
    }

    auto current = loops_->GetHeader(to);
    decltype(current) prev = nullptr;
    while (current)
    {
        if (ancfrom.count(current))
            break;
        prev = current;
        current = loops_->GetHeader(current);
    }
    return prev;
}


void Liveness::ExecuteOnFunction(Function* func)
{
    PartialLiveness(fg_->GetFlowGraph(), func->At(0));
    PropagateHeader(func);
}
