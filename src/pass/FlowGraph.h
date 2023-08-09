#ifndef _FLOW_GRAPH_H_
#define _FLOW_GRAPH_H_

#include "pass/Pass.h"
#include "utils/Graph.h"
#include <string>
#include <unordered_map>

class Module;
class Function;
class BasicBlock;
class IROperand;

class BrInstr;
class SwitchInstr;
class RetInstr;


class FlowGraph : public FunctionPass
{
public:
    using JumpCond = std::pair<const IROperand*, bool>;
    using GraphType = Graph<const BasicBlock*, JumpCond>;

    template <class ITER>
    class PredsIter
    {
    public:
        PredsIter(ITER b, ITER e) : begin_(b), end_(e) {}
        ITER begin() { return begin_; }
        ITER end() { return end_; }
    private:
        ITER begin_{};
        ITER end_{};
    };

    FlowGraph(Module* m) : FunctionPass(m) {}

    std::string PrintSummary() const override;

    void ExecuteOnFunction(Function*) override;
    void ExitFunction() override { flow_.Clear(); preds_.clear(); }

    const auto& GetFlowGraph() const { return flow_; }
    auto& GetFlowGraph() { return flow_; }

    const auto GetPredsOf(const BasicBlock* bb) const
    {
        auto [b, e] = preds_.equal_range(bb);
        return PredsIter(b, e);
    }
    auto GetPredsOf(const BasicBlock* bb)
    {
        auto [b, e] = preds_.equal_range(bb);
        return PredsIter(b, e);
    }

private:
    void VisitBasicBlock(BasicBlock*);
    void VisitBrInstr(BasicBlock*, BrInstr*);
    void VisitSwitchInstr(BasicBlock*, SwitchInstr*);
    void VisitRetInstr(BasicBlock*, RetInstr*);

    Graph<const BasicBlock*, JumpCond> flow_{};
    std::unordered_multimap<const BasicBlock*, const BasicBlock*> preds_{};
};

#endif // _FLOW_GRAPH_H_
