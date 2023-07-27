#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <algorithm>
#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>


template <class V, class C>
class __AdjListBase
{
public:
    __AdjListBase(std::unordered_map<V, int>& i,
        std::unordered_map<int, V*>& v) : indexof_(i), vertexvia_(v) {}

    bool HasLinkTo(const V& to) const { return vertices_.count(indexof_.at(to)); }
    bool HasLinkTo(int to) const { return vertices_.count(to); }
    void AddEdge(const V& to) { vertices_.insert(indexof_.at(to)); }
    void AddEdge(int to) { vertices_.insert(to); }
    void DeleteEdge(const V& to) { vertices_.erase(indexof_.at(to)); }
    void DeleteEdge(int to) { vertices_.insert(to); }

protected:
    C vertices_{};
    std::unordered_map<V, int>& indexof_;
    std::unordered_map<int, V*>& vertexvia_;
};


template <typename... T>
class AdjList;

template <typename V>
class AdjList<V> : public __AdjListBase<V, std::unordered_set<int>>
{
public:
    AdjList<V>(std::unordered_map<V, int>& i,
        std::unordered_map<int, V*>& v) : Base(i, v) {}

    bool operator[](const V& to) const { return HasLinkTo(to); }
    bool operator[](int to) const { return HasLinkTo(to); }
};


template <typename V, typename E>
class AdjList<V, E> : public __AdjListBase<V, std::unordered_multimap<int, E>>
{
    using Base = __AdjListBase<V, std::unordered_multimap<int, E>>;

public:
    template <class I>
    class Iterator
    {
    public:
        template <typename VERTEX, typename EDGE>
        struct Wrapper
        {
            VERTEX to_{};
            EDGE value_{};
        };

        using difference_type = ptrdiff_t;
        using value_type = V;
        using pointer = V*;
        using reference = V&;
        using iterator_category = std::bidirectional_iterator_tag;

        Iterator(I i, const std::unordered_map<int, V*>& m) : current_(i), map_(m) {}

        auto& operator++() { current_++; return *this; }
        auto operator++(int) { auto retval = *this; ++(*this); return retval; }
        auto& operator--() { current_--; return *this; }
        auto operator--(int) { auto retval = *this; --(*this); return retval; }

        auto& operator+=(size_t off) { *this = *this + off; return *this; }
        auto& operator-=(size_t off) { *this = *this - off; return *this; }
        auto operator+(size_t off) const { return Iterator(current_ + off, map_); }
        auto operator-(size_t off) const { return Iterator(current_ - off, map_); }

        bool operator==(Iterator<I> other) const { return current_ == other.current_; }
        bool operator!=(Iterator<I> other) const { return !(*this == other); }

        auto operator*()
        {
            return Wrapper<V&, E&>(
                *map_.at(current_->first), current_->second);
        }
        auto operator*() const
        {
            return Wrapper<const V&, const E&>(
                *map_.at(current_->first), current_->second);
        }

    private:
        I current_{};
        const std::unordered_map<int, V*>& map_;
    };

    template <class I, class CI>
    class Window
    {
    public:
        template <class I>
        class Iterator
        {
        public:
            using difference_type = ptrdiff_t;
            using value_type = V;
            using pointer = V*;
            using reference = V&;
            using iterator_category = std::bidirectional_iterator_tag;

            Iterator(I c) : current_(c) {}

            auto& operator++() { current_++; return *this; }
            auto operator++(int) { auto retval = *this; ++(*this); return retval; }
            auto& operator--() { current_--; return *this; }
            auto operator--(int) { auto retval = *this; --(*this); return retval; }

            auto operator+(size_t off) const { return Iterator(current_ + off, map_); }
            auto operator-(size_t off) const { return Iterator(current_ - off, map_); }

            bool operator==(Iterator<I> other) const { return current_ == other.current_; }
            bool operator!=(Iterator<I> other) const { return !(*this == other); }

            auto& operator*() { return current_->second; }
            const auto operator*() const { return current_->second; }

        private:
            I current_{};
        };

        Window(I b, I e) : begin_(b), end_(e) {}

        auto begin() { return Iterator(begin_); }
        auto end() { return Iterator(end_); }
        auto cbegin() const { return Iterator(cbegin_); }
        auto cend() const { return Iterator(cend_); }

    private:
        I begin_{};
        I end_{};
        CI cbegin_{};
        CI cend_{};
    };

    using MapIter = decltype(std::unordered_multimap<int, E>::equal_range(0));
    using ConstMapIter = const MapIter;
    using IterType = Iterator<MapIter>;
    using ConstIterType = Iterator<ConstMapIter>;

    AdjList<V, E>(std::unordered_map<V, int>& i,
        std::unordered_map<int, V*>& v) : Base(i, v) {}

    auto begin() { return Iterator(Base::vertices_.begin()); }
    auto end() { return Iterator(Base::vertices_.end()); }
    auto cbegin() const { return Iterator(Base::vertices_.cbegin()); }
    auto cend() const { return Iterator(Base::vertices_.cend()); }

    auto operator[](const V& to) const
    { return this->operator[](Base::indexof_.at(to)); }
    auto operator[](const V& to)
    { return this->operator[](Base::indexof_.at(to)); }
    auto operator[](int to) const
    {
        auto pair = Base::vertices_.equal_range(to);
        return Window(pair.first, pair.second);
    }
    auto operator[](int to)
    {
        auto pair = Base::vertices_.equal_range(to);
        return Window(pair.first, pair.second);
    }

    void AddValueEdge(const V& to, const E& value)
    { Base::vertices_.emplace(Base::indexof_[to], value); }

    void DeleteValueEdge(const V& to, const E& value)
    {
        auto range = Base::vertices_.equal_range(to);
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == value)
            {
                Base::vertices_.erase(it);
                break;
            }
        }
    }
};


template <typename V>
struct Vertex
{
    V* vertex_{};
    int finish_{};
    int discover_{};
    bool visited_{};
};

template <typename V, class ADJ>
class __GraphBase
{
public:
    class ItemIterator
    {
    public:
        using difference_type = ptrdiff_t;
        using value_type = std::pair<Vertex<V>&, ADJ&>;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::random_access_iterator_tag;

        ItemIterator(Vertex<V>* v, ADJ* a, int i) : vertices_(v), adjlist_(a), index_(i) {}

        auto& operator++() { index_++; return *this; }
        auto operator++(int) { auto retval = *this; ++(*this); return retval; }
        auto& operator--() { index_--; return *this; }
        auto operator--(int) { auto retval = *this; --(*this); return retval; }

        auto& operator+=(size_t off) { index_ += off; return *this; }
        auto& operator-=(size_t off) { index_ -= off; return *this; }
        auto operator+(size_t off) const { auto retval = *this; return retval += off; }
        auto operator-(size_t off) const { auto retval = *this; return retval -= off; }
        size_t operator+(const ItemIterator& other) const { return index_ + other.index_; }
        size_t operator-(const ItemIterator& other) const { return index_ - other.index_; }

        bool operator==(const ItemIterator& other) const
        {
            return vertices_ == other.vertices_ &&
                adjlist_ == other.adjlist_ && index_ == other.index_;
        }
        bool operator!=(const ItemIterator& other) const { return !(*this == other); }

        std::pair<Vertex<V>&, ADJ&> operator*()
        {
            return std::make_pair<
                Vertex<V>&, ADJ&>(vertices_[index_], adjlist_[index_]);
        }
        std::pair<const Vertex<V>&, const ADJ&> operator*() const
        {
            return std::make_pair<
                const Vertex<V>&, const ADJ&>(vertices_[index_], adjlist_[index_]);
        }

    private:
        Vertex<V>* vertices_{};
        ADJ* adjlist_{};
        int index_{};
    };

    auto begin() { return ItemIterator(vertexvia_.data(), adj_.data(), 0); }
    auto end() { return ItemIterator(vertexvia_.data(), adj_.data(), vertexvia_.size()); }
    const auto cbegin() const { return begin(); }
    const auto cend() const { return end(); }

    auto& GetVertices() { return vertexvia_; }
    const auto& GetVertices() const { return vertexvia_; }
    auto& GetEdges(const V& v) { return adj_[indexof_[v]]; }
    const auto& GetEdges(const V& v) const { return adj_[indexof_[v]]; }
    auto& operator[](const V& v) { return GetEdges(v); }
    const auto& operator[](const V& v) const { return GetEdges(v); }

    void AddVertex(const V& v)
    {
        auto addr = &(indexof_.emplace(v, index_).first->first);
        vertexvia_[index_] = { addr, 0, 0, false };
        adj_.push_back({});
        index_ += 1;
    }

    void DeleteVertex(const V& v)
    {
        int i = indexof_[v];
        for (int j = 0; j < vertexvia_.size(); ++j)
        {
            if (j == i) continue;
            if (adj_[j].HasLinkTo(i))
                adj_[j].DeleteEdge(i);
        }
        indexof_.erase(v);
    }

    void AddEdge(const V& from, const V& to)
    {
        int ifrom = indexof_[from];
        int ito = indexof_[to];
        adj_[ifrom].AddEdge(ito);
    }

    void DeleteEdge(const V& from, const V& to)
    {
        int ifrom = indexof_[from];
        int ito = indexof_[to];
        adj_[ifrom].DeleteEdge(ito);
    }

protected:
    int index_{};
    std::unordered_map<V, int> indexof_{};
    std::vector<Vertex<V>> vertexvia_{};
    std::vector<ADJ> adj_{};
};

template <typename... T>
class Graph;

template <typename V>
class Graph<V> : public __GraphBase<V, AdjList<V>> {};

template <typename V, typename E>
class Graph<V, E> : public __GraphBase<V, AdjList<V, E>>
{
    using Base = __GraphBase<V, AdjList<V, E>>;

public:
    void AddValueEdge(const V& from, const V& to, const E& value)
    {
        int ifrom = Base::indexof_[from];
        Base::adj_[ifrom].AddValueEdge(to, value);
    }

    void DeleteValueEdge(const V& from, const V& to, const E& value)
    {
        int ifrom = Base::indexof_[from];
        Base::adj_[ifrom].DeleteValueEdge(to, value);
    }
};

#endif // _GRAPH_H_
