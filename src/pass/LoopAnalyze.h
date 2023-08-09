#ifndef _LOOP_ANALYZE_H_
#define _LOOP_ANALYZE_H_

#include "pass/Pass.h"
#include "pass/FlowGraph.h"
#include <string>
#include <unordered_map>
#include <vector>

class Module;
class Function;
class BasicBlock;


// Identify loops in a possibly irreducible CFG and mark
// the headers of natural loops and irreducible loops.
// The implementation below is based on A New Algorithm for
// Identifying Loops in Decompilation by Wei et al. (2007).
// See https://lenx.100871.net/papers/loop-SAS.pdf for more information.

class LoopAnalyze : public FunctionPass
{
public:
    LoopAnalyze(Module* m, Pass* c) :
        FunctionPass(m), fg_(static_cast<FlowGraph*>(c)) {}

    std::string PrintSummary() const override;

    void ExecuteOnFunction(Function* func) override { CurFunc() = func; IdentifyLoops(func); }
    void ExitFunction() override { loopinfo_.clear(); }

    const BasicBlock* GetHeader(const BasicBlock* bb) const { return loopinfo_.at(bb).loopheader_; }
    bool InLoop(const BasicBlock* bb) const { return loopinfo_.at(bb).loopheader_; }
    bool IsHeader(const BasicBlock* bb) const { return loopinfo_.at(bb).isheader_; }
    bool InIrreducible(const BasicBlock*) const;
    bool IsIrreducible(const BasicBlock* bb) const { return loopinfo_.at(bb).isirreducible_; }
    bool IsReenrty(const BasicBlock* from, const BasicBlock* to) const;

private:
    void IdentifyLoops(const Function*);
    const BasicBlock* DFS(const FlowGraph::GraphType&, const BasicBlock*, int);
    void MarkLoopHeader(const BasicBlock*, const BasicBlock*);

    struct LoopInfo
    {
        bool visited_{};
        bool isheader_{};
        bool isirreducible_{};
        // block's POSition in Deep First Search Path
        int dfsppos_{};
        const BasicBlock* loopheader_{};
    };

    std::unordered_map<const BasicBlock*, LoopInfo> loopinfo_{};
    FlowGraph* fg_{};
};

#endif // _LOOP_ANALYZE_H_
