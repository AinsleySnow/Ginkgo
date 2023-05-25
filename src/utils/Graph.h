#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <iterator>
#include <unordered_map>


template <typename T>
class Vertex
{
public:
    Vertex() {}
    Vertex(T data) : data_(data) {}

    T& Data() { return data_; }
    T Data() const { return data_; }
    void Reset() { visited_ = false; f_ = d_ = 0; }

    bool operator==(const Vertex& rhs) const { return data_ == rhs.data_; }
    bool operator!=(const Vertex& rhs) const { return !(*this == rhs); }
    bool operator<(const Vertex& rhs) const { return data_ < rhs.data_; }
    bool operator>(const Vertex& rhs) const { return data_ > rhs.data_; }

private:
    T data_{};

    bool visited_{ false };
    long f_{}, d_{};
};


template <typename V, typename VALUE = int>
class Edge
{
public:
    Edge(Vertex<V>& from, Vertex<V>& to) : from_(from), to_(to) {}
    Edge(Vertex<V>& from, Vertex<V>& to, VALUE value) : from_(from), to_(to), value_(value) {}

    auto& From() { return from_; }
    const auto& From() const { return from_; }
    auto& To() { return to_; }
    const auto& To() const { return to_; }
    VALUE& Value() { return value_; }
    VALUE Value() const { return value_; }

    bool operator==(const Edge& rhs) const
    {
        return from_ == rhs.from_ &&
            to_ == rhs.to_ && value_ == rhs.value_;
    }
    bool operator!=(const Edge& rhs) const { return !(*this == rhs); }

private:
    Vertex<V>& from_;
    Vertex<V>& to_;
    VALUE value_{};
};


template <typename V, typename VALUE = int>
class AdjacencyList
{
public:
    using VertexType = Vertex<V>;
    using EdgeType = Edge<VertexType, VALUE>;
    using EdgeList = decltype(
        std::declval<std::unordered_multimap<VertexType, EdgeType>>()
            .equal_range(std::declval<VertexType>()));

    class EdgeVisitor
    {
    public:
        EdgeVisitor(EdgeList&& edges) : edges_(edges) {}
        auto& operator[](int i) { return std::advance(edges_.first, i); }
        auto begin() { return edges_.first; }
        auto end() { return edges_.second; }
        const auto begin() const { return edges_.first; }
        const auto end() const { return edges_.second; }

    private:
        EdgeList edges_;
    };

    void AddEdge(const EdgeType& edge) { edges_.push_back(edge); }
    void AddEdge(EdgeType&& edge) { edges_.push_back(std::move(edge)); }

    auto operator[](const VertexType& v) { return EdgeVisitor(edges_.equal_range(v)); }

    auto GetEdgeValue(const VertexType& to)
    {
        auto range = edges_.equal_range(to);
        for (auto it = range.first; it != range.second; ++it)
            if (it->To() == v)
                return it->Value();
        return VALUE{};
    }
    void SetEdgeValue(const VertexType& to, const VALUE& value)
    {
        auto& range = edges_.equal_range(to);
        range.first->Value() = value;
    }

private:
    std::unordered_multimap<VertexType, EdgeType> edges_{};
};


template <typename V, typename E>
class Graph
{
public:
    using VertexType = Vertex<V>;
    using EdgeType = Edge<VertexType, E>;
    using AdjList = AdjacencyList<V, E>;

    void ResetAll() { for (auto& [v, _] : graph_) v.Reset(); }

    void AddVertex(const VertexType& v) { graph_.insert({ v, {} }); }
    void AddEdge(const VertexType& from, const VertexType& to)
    { graph_.insert({ from, { to, { from, to } } }); }
    void AddEdge(const VertexType& from, const VertexType& to, const E& value)
    { graph_.insert({ from, { to, { from, to, value } } }); }

    auto GetEdgeValue(const VertexType& from, const VertexType& to, int index = 0)
    {
        auto& adj = graph_.at(from);
        return adj[to][index].Value();
    }
    void SetEdgeValue(const VertexType& from, const VertexType& to, const E& value)
    {
        auto& adj = graph_.at(from);
        adj[to][0].Value() = value;
    }

    auto& operator[](const VertexType& v) { return graph_[v]; }

private:
    std::unordered_map<VertexType, AdjList> graph_{};
};

#endif // _GRAPH_H_
