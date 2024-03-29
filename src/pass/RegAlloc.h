#ifndef _REG_ALLOC_H_
#define _REG_ALLOC_H_

#include "visitir/IRVisitor.h"
#include <set>

class Function;


// the reason that I use a template here is that, what if someone
// wants to implement a register allocator for another architecture?
template <typename REGENUM, typename ARCHINFO>
class RegAlloc
{
protected:
    void Mark(REGENUM r)         { inuse_.insert(r); used_.insert(r); }
    void Unmark(REGENUM r)       { inuse_.erase(r); }
    bool InUse(REGENUM r) const  { return inuse_.find(r) != inuse_.end(); }

    ARCHINFO& ArchInfo() { return curfunc_; }
    const ARCHINFO& ArchInfo() const { return curfunc_; }
    auto& UsedRegs() const { return used_; }

    void Clear() { inuse_ = {}; used_ = {}; }

private:
    std::set<REGENUM> inuse_{};
    std::set<REGENUM> used_{};
    ARCHINFO curfunc_{};
};


#endif // _REG_ALLOC_H_
