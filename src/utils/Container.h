#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <algorithm>
#include <memory>
#include <vector>


template <class ELE>
class Container
{
public:
    class Iterator: public std::iterator<
        std::random_access_iterator_tag, ELE*, ptrdiff_t,
        std::unique_ptr<ELE>*,
        std::unique_ptr<ELE>&>
    {
        using ptr = std::unique_ptr<ELE>;

    public:
        Iterator(ptr* p, size_t i = 0) : start_(p), index_(i) {}

        Iterator& operator++() { index_++; return *this; }
        Iterator operator++(int) { Iterator retval = *this; ++(*this); return retval; }
        Iterator& operator--() { index_--; return *this; }
        Iterator operator--(int) { Iterator retval = *this; --(*this); return retval; }

        Iterator& operator+=(size_t off) { index_ += off; return *this; }
        Iterator& operator-=(size_t off) { index_ -= off; return *this; }
        Iterator operator+(size_t off) const { Iterator retval = *this; return retval += off; }
        Iterator operator-(size_t off) const { Iterator retval = *this; return retval -= off; }
        size_t operator+(Iterator other) const { return index_ + other.index_; }
        size_t operator-(Iterator other) const { return index_ - other.index_; }

        bool operator==(Iterator other) const
        {
            return start_ == other.start_ &&
                index_ == other.index_;
        }
        bool operator!=(Iterator other) const { return !(*this == other); }

        ELE* operator*() const { return start_[index_].get(); }

    private:
        ptr* start_{};
        size_t index_{};
    };

    auto begin() { return Iterator(elements_.data()); }
    auto end() { return Iterator(elements_.data(), elements_.size()); }

    void Append(std::unique_ptr<ELE> ele)
    {
        elements_.push_back(std::move(ele));
    }
    void Insert(int i, std::unique_ptr<ELE> ele)
    {
        elements_.insert(elements_.begin() + i,
            std::move(ele));
    }
    void Remove() { elements_.pop_back(); }
    void Remove(int i) { elements_.erase(elements_.begin() + i); }

    bool Empty() const { return elements_.empty(); }
    auto Size() const { return elements_.size(); }

    ELE* At(int i) { return elements_[i].get(); }
    int IndexOf(const ELE* ptr) const
    {
        auto begin = std::make_move_iterator(elements_.begin());
        auto end = std::make_move_iterator(elements_.end());
        auto pos = std::find_if(begin, end,
            [ptr] (const auto& ele) { return ptr == ele.get(); });
        return pos == end ? -1 : std::distance(begin, pos);
    }


protected:
    std::vector<std::unique_ptr<ELE>> elements_{};
};

#endif // _CONTAINER_H_
