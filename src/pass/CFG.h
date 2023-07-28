#ifndef _CFG_H_
#define _CFG_H_

#include "pass/Pass.h"
#include "utils/Graph.h"
#include <unordered_map>
#include <unordered_set>

class Module;
class Function;
class BasicBlock;
class IROperand;

class BrInstr;
class CallInstr;
class SwitchInstr;
class RetInstr;


class CFG : public ModulePass
{
public:
    using JumpCond = std::pair<const IROperand*, bool>;
    using CallInfo = std::pair<const BasicBlock*, const CallInstr*>;
    using FlowGraph = Graph<const BasicBlock*, JumpCond>;
    using CallingGraph = Graph<const Function*, CallInfo>;
    using Predecessors = std::unordered_set<const BasicBlock*>;

    CFG(Module* m) : ModulePass(m) {}
    void ExecuteOnModule(Module*) override;

    const CallingGraph& GetCallingGraph() const { return calling_; }
    CallingGraph& GetCallingGraph() { return calling_; }

    const FlowGraph& GetFlowGraph(const Function* func) const { return flow_.at(func); }
    FlowGraph& GetFlowGraph(const Function* func) { return flow_.at(func); }

    const Predecessors& GetPredsOf(const BasicBlock* bb) const { return preds_.at(bb); }
    Predecessors& GetPredsOf(const BasicBlock* bb) { return preds_.at(bb); }

private:
    void VisitFunction(Function*);
    void VisitBasicBlock(Function*, BasicBlock*);
    void VisitBrInstr(Function*, BasicBlock*, BrInstr*);
    void VisitCallInstr(Function*, BasicBlock*, CallInstr*);
    void VisitSwitchInstr(Function*, BasicBlock*, SwitchInstr*);
    void VisitRetInstr(Function*, BasicBlock*, RetInstr*);

    CallingGraph calling_{};
    std::unordered_map<const Function*, FlowGraph> flow_{};
    std::unordered_map<const BasicBlock*, Predecessors> preds_{};
};

#endif // _CFG_H_
