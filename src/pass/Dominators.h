#ifndef _DOMINATORS_H_
#define _DOMINATORS_H_

#include "pass/Pass.h"
#include "pass/CFG.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Module;


// Find dominators of basic blocks. The implementation here is based
// on A Simple, Fast Dominance Algorithm by Cooper, Harvey & Kennedy (2006).
// See https://www.researchgate.net/publication/2569680_A_Simple_Fast_Dominance_Algorithm
// for more information.

class Dominators : public FunctionPass
{
public:
    using Domins = std::unordered_map<
        const BasicBlock*, std::unordered_set<const BasicBlock*>>;

    Dominators(Module* m, Pass* cfg) :
        FunctionPass(m), graphs_(static_cast<CFG*>(cfg)) {}

    void ExecuteOnFunction(Function*) override;
    void ExitFunction(Function*) override;

    const Domins& GetDominators(const Function* func) const { return domins_.at(func); }

private:
    void DFS(const CFG::FlowGraph&, const BasicBlock*, int&);
    void MapPostorder(const Function*);
    void FindIDom(const Function*);
    int Intersect(int, int);
    void ConstructDoms(const Function*);

    std::vector<const BasicBlock*> bbvia_{};
    std::unordered_map<const BasicBlock*, int> indexof_{};
    std::vector<int> idom_{};

    std::unordered_map<const Function*, Domins> domins_{};
    CFG* graphs_{};
};

#endif // _DOMINATORS_H_
