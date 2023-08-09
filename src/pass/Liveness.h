#ifndef _LIVENESS_H_
#define _LIVENESS_H_

#include "pass/Pass.h"
#include "pass/FlowGraph.h"
#include "pass/DUInfo.h"
#include "pass/LoopAnalyze.h"
#include <string>
#include <unordered_set>

class Module;
class Function;
class BasicBlock;


// Do liveness analysis on SSA IR. The implementation
// below is based on the algorithm introduced in section
// 9.2 in SSA-based Compiler Design. 
// See https://link.springer.com/book/10.1007/978-3-030-80515-9
// for more information.

class Liveness : public FunctionPass
{
public:
    Liveness(Module* m, Pass* g, Pass* du, Pass* l) :
        FunctionPass(m), fg_(static_cast<FlowGraph*>(g)),
        duinfo_(static_cast<DUInfo*>(du)),
        loops_(static_cast<LoopAnalyze*>(l)) {}

    std::string PrintSummary() const override;

    void ExecuteOnFunction(Function*) override;
    void ExitFunction() override { visited_.clear(); livein_.clear(); liveout_.clear(); }

    const auto& LiveIn(const BasicBlock* bb) { return livein_[bb]; }
    const auto& LiveOut(const BasicBlock* bb) { return liveout_[bb]; }
    bool LiveInAt(const IROperand* op, const BasicBlock* bb) { return livein_[bb].count(op); }
    bool LiveOutAt(const IROperand* op, const BasicBlock* bb) { return liveout_[bb].count(op); }

private:
    void PartialLiveness(const FlowGraph::GraphType&, const BasicBlock*);
    void PropagateHeader(const Function*);
    // Find outermost excluding loop. That is,
    // the highest loop header containing 'to' but not 'from'.
    const BasicBlock* FindOLE(const BasicBlock* from, const BasicBlock* to) const;

    bool OnPath(const BasicBlock* b) const { return onpath_.count(b); }
    bool Visited(const BasicBlock* b) const { return visited_.count(b); }

    std::unordered_set<const BasicBlock*> onpath_{};
    std::unordered_set<const BasicBlock*> visited_{};

    mutable std::unordered_map<const BasicBlock*,
        std::unordered_set<const IROperand*>> livein_{};
    mutable std::unordered_map<const BasicBlock*,
        std::unordered_set<const IROperand*>> liveout_{};

    FlowGraph* fg_{};
    DUInfo* duinfo_{};
    LoopAnalyze* loops_{};
};

#endif // _LIVENESS_H_
