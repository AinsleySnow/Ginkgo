#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <algorithm>
#include <memory>
#include <vector>


template <class ELE>
class Container
{
public:
    template <typename T, typename V>
    class Iterator
    {
        using ptr = T;

    public:
        using difference_type = ptrdiff_t;
        using value_type = V;
        using pointer = ptr*;
        using reference = ptr&;
        using iterator_category = std::random_access_iterator_tag;

        Iterator(ptr* p, size_t i = 0) : start_(p), index_(i) {}

        auto& operator++() { index_++; return *this; }
        auto operator++(int) { auto retval = *this; ++(*this); return retval; }
        auto& operator--() { index_--; return *this; }
        auto operator--(int) { auto retval = *this; --(*this); return retval; }

        auto& operator+=(size_t off) { index_ += off; return *this; }
        auto& operator-=(size_t off) { index_ -= off; return *this; }
        auto operator+(size_t off) const { auto retval = *this; return retval += off; }
        auto operator-(size_t off) const { auto retval = *this; return retval -= off; }
        size_t operator+(Iterator<T, V> other) const { return index_ + other.index_; }
        size_t operator-(Iterator<T, V> other) const { return index_ - other.index_; }

        bool operator==(Iterator<T, V> other) const
        {
            return start_ == other.start_ &&
                index_ == other.index_;
        }
        bool operator!=(Iterator<T, V> other) const { return !(*this == other); }

        V* operator*() { return start_[index_].get(); }
        V* operator*() const { return start_[index_].get(); }

    private:
        ptr* start_{};
        size_t index_{};
    };

    using IterType = Iterator<std::unique_ptr<ELE>, ELE>;
    using ConstIterType = Iterator<const std::unique_ptr<ELE>, const ELE>;

    auto begin() { return IterType(elements_.data()); }
    auto end() { return IterType(elements_.data(), elements_.size()); }
    const auto begin() const { return ConstIterType(elements_.data()); }
    const auto end() const { return ConstIterType(elements_.data(), elements_.size()); }

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

    ELE* Front() { return elements_.front().get(); }
    const ELE* Front() const { return elements_.front().get(); }
    ELE* Back() { return elements_.back().get(); }
    const ELE* Back() const { return elements_.back().get(); }
    ELE* At(int i) { return elements_[i].get(); }
    const ELE* At(int i) const { return elements_[i].get(); }

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
