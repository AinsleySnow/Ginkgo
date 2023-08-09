#ifndef _CALLING_GRAPH_H_
#define _CALLING_GRAPH_H_

#include "pass/Pass.h"
#include "utils/Graph.h"
#include <string>
#include <unordered_map>

class BasicBlock;
class Function;
class Module;
class CallInstr;


class CallingGraph : public ModulePass
{
public:
    using CallInfo = std::pair<const BasicBlock*, const CallInstr*>;
    using GraphType = Graph<const Function*, CallInfo>;

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

    CallingGraph(Module* m) : ModulePass(m) {}
    std::string PrintSummary() const override;
    void ExecuteOnModule() override;

    const auto& GetCallingGraph() const { return calling_; }
    auto& GetCallingGraph() { return calling_; }
    const auto GetPredsOf(const Function* func) const
    {
        auto [b, e] = preds_.equal_range(func);
        return PredsIter(b, e);
    }
    auto GetPredsOf(const Function* func)
    {
        auto [b, e] = preds_.equal_range(func);
        return PredsIter(b, e);
    }

private:
    void VisitFunction(Function*);
    void VisitCallInstr(Function*, CallInstr*);

    Graph<const Function*, CallInfo> calling_{};
    std::unordered_multimap<const Function*, const Function*> preds_{};
};

#endif // _CALLING_GRAPH_H_
