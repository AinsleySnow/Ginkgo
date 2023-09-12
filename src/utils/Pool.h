#ifndef _POOL_H_
#define _POOL_H_

#include <memory>
#include <vector>


template <class T>
class Pool
{
public:
    void Add(std::unique_ptr<T> t) { pool_.push_back(std::move(t)); }
    void Clear() { pool_.clear(); }
    void Merge(Pool<T>* pool)
    {
        pool_.insert(pool_.end(),
            std::make_move_iterator(pool->pool_.begin()),
            std::make_move_iterator(pool->pool_.end()));
        pool->Clear();
    }

private:
    std::vector<std::unique_ptr<T>> pool_{};
};

#endif // _POOL_H_
