#ifndef _LIVENESS_H_
#define _LIVENESS_H_

#include "pass/Pass.h"
#include "pass/FlowGraph.h"
#include "pass/DUInfo.h"
#include "pass/LoopAnalyze.h"
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
        FunctionPass(m), cfg_(static_cast<CFG*>(g)),
        duinfo_(static_cast<DUInfo*>(du)),
        loops_(static_cast<LoopAnalyze*>(l)) {}

    void ExecuteOnFunction(Function*) override;
    void ExitFunction(Function*) override { visited_.clear(); }

    const auto& LiveIn(const BasicBlock* bb) const { return livein_.at(bb); }
    const auto& LiveOut(const BasicBlock* bb) const { return liveout_.at(bb); }

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

    std::unordered_map<const BasicBlock*,
        std::unordered_set<const IROperand*>> livein_{};
    std::unordered_map<const BasicBlock*,
        std::unordered_set<const IROperand*>> liveout_{};

    FlowGraph* fg_{};
    DUInfo* duinfo_{};
    LoopAnalyze* loops_{};
};

#endif // _LIVENESS_H_
