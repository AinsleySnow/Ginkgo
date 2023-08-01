#include "pass/Dominators.h"
#include "utils/Graph.h"


void Dominators::DFS(const CFG::FlowGraph& fg, const BasicBlock* bb, int& poi)
{
    for (auto [to, _] : fg[bb])
        // not present in the set
        if (!indexof_.count(to))
            DFS(fg, to, poi);
    indexof_[bb] = poi;
    bbvia_.push_back(bb);
    poi += 1;
}

void Dominators::MapPostorder(const Function* func)
{
    auto& fg = graphs_->GetFlowGraph(func);
    int poi = 0; // post-order index

    for (auto& v : fg.GetVertices())
        if (auto vv = *v; !indexof_.count(vv))
            DFS(fg, vv, poi);

    idom_.reserve(poi);
    std::fill_n(std::back_inserter(idom_), poi, -1);
}

void Dominators::FindIDom(const Function* func)
{
    int start = indexof_[func->At(0)];

    const auto& flow = graphs_->GetFlowGraph(func);
    auto defined = [this] (int po) -> bool {
        return idom_[po] != -1;
    };

    idom_[start] = start;
    bool changing = true;
    while (changing)
    {
        changing = false;
        // Except the start node (the node with the biggest postorder number)
        for (auto i = bbvia_.size() - 1; i >= 0; --i)
        {
            int newidom = -1;
            auto node = bbvia_[i];
            // Pick some processed predecessor of the current block
            for (auto& pred : graphs_->GetPredsOf(node))
            {
                if (auto p = indexof_[pred]; defined(p))
                {
                    newidom = p;
                    break;
                }
            }
            for (auto& pred : graphs_->GetPredsOf(node))
                if (auto p = indexof_[pred]; defined(p))
                    newidom = Intersect(p, newidom);
            if (idom_[i] != newidom)
            {
                idom_[i] = newidom;
                changing = true;
            }
        }
    }
}

int Dominators::Intersect(int b1, int b2)
{
    int finger1 = b1, finger2 = b2;
    while (finger1 != finger2)
    {
        while (finger1 < finger2)
            finger1 = idom_[finger1];
        while (finger2 < finger1)
            finger2 = idom_[finger2];
    }
    return finger1;
}

void Dominators::ConstructDoms(const Function* func)
{
    auto start = func->At(0);
    domins_.emplace(start, start);
    for (auto& [node, _] : indexof_)
    {
        for (auto current = node; current != start; )
        {
            domins_.emplace(node, current);
            current = bbvia_[idom_[indexof_[current]]];
        }
    }
}


void Dominators::ExecuteOnFunction(Function* func)
{
    MapPostorder(func);
    FindIDom(func);
}

void Dominators::ExitFunction(Function* func)
{
    indexof_.clear();
    bbvia_.clear();
    idom_.clear();
}
