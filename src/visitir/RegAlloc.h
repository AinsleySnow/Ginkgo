#ifndef _REG_ALLOC_H_
#define _REG_ALLOC_H_

#include "visitir/IRVisitor.h"
#include <unordered_map>
#include <unordered_set>

class Function;


// the reason that I use a template here is that, what if someone
// wants to implement a register allocator for another architecture?
template <typename REGENUM, typename ARCHINFO>
class RegAlloc : protected IRVisitor
{
protected:
    void Mark(REGENUM r)         { inuse_.insert(r); used_.insert(r); }
    void Unmark(REGENUM r)       { inuse_.erase(r); }
    bool InUse(REGENUM r) const  { return inuse_.find(r) != inuse_.end(); }
    void ClearReg() { inuse_.clear(); used_.clear(); curfunc_ = {}; }

    ARCHINFO& ArchInfo() { return curfunc_; }
    const ARCHINFO& ArchInfo() const { return curfunc_; }

private:
    std::unordered_set<REGENUM> inuse_{};
    std::unordered_set<REGENUM> used_{};
    ARCHINFO curfunc_{};
};

#endif // _REG_ALLOC_H_
