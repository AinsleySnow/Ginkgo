#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include <memory>
#include <vector>


template <class T>
class MemPool
{
public:
    void Add(std::unique_ptr<T> t) { pool_.push_back(std::move(t)); }
    void Merge(MemPool<T>* pool)
    {
        pool_.insert(pool_.end(),
            std::make_move_iterator(pool->pool_.begin()),
            std::make_move_iterator(pool->pool_.end()));
    }

private:
    std::vector<std::unique_ptr<T>> pool_{};
};

#endif // _MEM_POOL_H_
