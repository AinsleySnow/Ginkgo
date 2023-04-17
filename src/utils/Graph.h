#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <queue>
#include <unordered_map>


template <typename T>
class Vertex
{
public:
    Vertex() {}
    Vertex(T data) : data_(data) {}

    T& Data() { return data_; }
    T Data() const { return data_; }

    void Reset()
    {
        visited_ = false;
        f_ = d_ = 0;
    }

private:
    T data_{};

    bool visited_{ false };
    long f_{}, d_{};
};


template <typename VERTEX, typename VALUE = int>
class Edge
{
public:
    Edge() {}

    auto& To() { return to_; }
    const auto& To() const { return to_; }
    VALUE& Value() { return value_; }
    VALUE Value() const { return value_; }

private:
    VERTEX& to_{};
    VALUE value_{};
};


template <typename V, typename E>
class Graph
{
public:
    using VertexType = Vertex<V>;
    using EdgeType = Edge<VertexType, E>;
    using AdjList = std::unordered_multimap<VertexType, EdgeType>;

    void ResetAll() { for (auto& [v, _] : graph_) v.Reset(); }

private:
    std::unordered_map<VertexType, AdjList> graph_{};
};

#endif // _GRAPH_H_
 