#include "pass/LoopAnalyze.h"
#include "utils/Graph.h"


void LoopAnalyze::IdentifyLoops(const Function* func)
{
    auto fg = cfg_->GetFlowGraph(func);
    for (auto& v : fg.GetVertices())
        loopinfo_.emplace(*v, (LoopInfo){});
    DFS(fg, func->At(0), 1);
}

const BasicBlock* LoopAnalyze::DFS(
    const CFG::FlowGraph& fg, const BasicBlock* b0, int pos)
{
    loopinfo_[b0].dfsppos_ = pos;
    loopinfo_[b0].visited_ = true;

    for (auto [b, _] : fg[b0])
    {
        // Not traversed?
        if (loopinfo_[b].visited_ == false)
        {
            auto nh = DFS(fg, b, pos + 1);
            MarkLoopHeader(b0, nh);
            continue;
        }

        auto h = loopinfo_[b].loopheader_;
        // We have encountered b before
        if (loopinfo_[b].dfsppos_ > 0) // b in DFSP(b0)
        {
            loopinfo_[b].isheader_ = true;
            MarkLoopHeader(b0, b);
        }
        else if (loopinfo_[b].loopheader_ == nullptr)
            continue;
        else // h not in DFSP(b0); re-entry
        {
            loopinfo_[h].isirreducible_ = true;
            while (loopinfo_[h].loopheader_)
            {
                h = loopinfo_[h].loopheader_;
                if (loopinfo_[h].dfsppos_ > 0)
                {
                    MarkLoopHeader(b0, h);
                    break;
                }
                loopinfo_[h].isirreducible_ = true;
            }
        }
    }
    loopinfo_[b0].dfsppos_ = 0;
    return loopinfo_[b0].loopheader_;
}

void LoopAnalyze::MarkLoopHeader(const BasicBlock* b, const BasicBlock* h)
{
    if (b == h || h == nullptr)
        return;
    auto cur1 = b, cur2 = h;
    while (auto ih = loopinfo_[cur1].loopheader_)
    {
        if (ih == cur2)
            return;
        if (loopinfo_[ih].dfsppos_ < loopinfo_[cur2].dfsppos_)
        {
            loopinfo_[cur1].loopheader_ = cur2;
            cur1 = cur2;
            cur2 = ih;
        }
        else
            cur1 = ih;
    }
    loopinfo_[cur1].loopheader_ = cur2;
}


bool LoopAnalyze::InIrreducible(const BasicBlock* bb) const
{
    auto header = loopinfo_.at(bb).loopheader_;
    if (!header)
        return false;
    if (loopinfo_.at(bb).isheader_)
        return false;
    return loopinfo_.at(header).isirreducible_;
}

bool LoopAnalyze::IsReenrty(
    const BasicBlock* from, const BasicBlock* to) const
{
    //      b0--+
    //       v  |
    //   +->b1<-+       b1 is the header of an irreducible loop.
    //   |   v  |       The re-entry edge is b0 -> b2.
    //   |->b2<-+       Edge b0 -> b1, b3 -> b2 and b3 -> b1
    //   |   v          are not re-entry edges.
    //   +--b3
    if (!InIrreducible(to) || IsHeader(to))
        return false;
    if (loopinfo_.at(from).loopheader_ == loopinfo_.at(to).loopheader_)
        return false;
    return true;
}
