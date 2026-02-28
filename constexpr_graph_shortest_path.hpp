#pragma once

#include <array>
#include <algorithm>
#include <cstddef>
#include <concepts>
#include <span>

template <typename EdgeType>
concept EdgeConcept = requires(EdgeType e) {
    { e.src };
    { e.dst };
    requires std::same_as<std::remove_cvref_t<decltype(e.src)>, std::remove_cvref_t<decltype(e.dst)>>;
    requires std::is_trivially_copyable_v<EdgeType>;
    requires std::equality_comparable<EdgeType>;
};

template <typename EdgeType>
requires EdgeConcept<EdgeType>
using NodeType = std::remove_cvref_t<decltype(EdgeType::src)>;

// ---------- constexpr queue ----------
template <typename T, size_t capacity>
struct ConstexprQueue {
    std::array<T, capacity> buf{};
    size_t head = 0, tail = 0, sz = 0;

    constexpr bool empty() const { return sz == 0; }
    constexpr void push(const T& v) {
        buf[tail] = v;
        tail = (tail + 1) % capacity;
        ++sz;
    }
    constexpr T pop() {
        T v = buf[head];
        head = (head + 1) % capacity;
        --sz;
        return v;
    }
};

// ---------- adjacency builder ----------
template <typename EdgeType, size_t num_edges, size_t node_count>
    requires EdgeConcept<EdgeType>
constexpr auto adjacency_list(const std::array<EdgeType, num_edges>& edges) {
    std::array<std::array<size_t, num_edges>, node_count> out{};
    std::array<size_t, node_count> counts{};
    for (size_t i = 0; i < num_edges; ++i) {
        NodeType<EdgeType> u = edges[i].src;
        out[u][counts[u]++] = i;
    }
    return std::make_pair(out, counts);
}

// ---------- result ----------
template <typename EdgeType, size_t max_edges>
    requires EdgeConcept<EdgeType>
struct BFSResult {
    std::array<EdgeType, max_edges> path{};
    size_t length = 0;
    bool found = false;

    constexpr auto view() const {
        return std::span(path.data(), length);
    }

    // comparison operator for edge sequence (check for path equality)
    template <size_t route_length>
    constexpr bool is_equal(const std::array<EdgeType, route_length>& other) const {
        static_assert(route_length <= max_edges, "Route length exceeds maximum edges");
        if (length != route_length) return false;
        for (size_t i = 0; i < length; ++i) {
            if (path[i] != other[i]) {
                return false;
            }
        }
        return true;
    }
};

// ---------- constexpr BFS ----------
template <size_t num_nodes, typename EdgeType, size_t num_edges>
    requires EdgeConcept<EdgeType>
constexpr BFSResult<EdgeType, num_nodes - 1>
bfs_find_shortest_path(const std::array<EdgeType, num_edges>& edges, NodeType<EdgeType> start, NodeType<EdgeType> goal) {
    auto [adj, counts] = adjacency_list<EdgeType, num_edges, num_nodes>(edges);
    std::array<int, num_nodes> prev{};
    std::array<int, num_nodes> via_edge{};
    std::array<bool, num_nodes> visited{};

    for (size_t i = 0; i < num_nodes; ++i) {
        prev[i] = -1;
        via_edge[i] = -1;
        visited[i] = false;
    }

    ConstexprQueue<NodeType<EdgeType>, num_nodes> q{};
    q.push(start);
    visited[start] = true;

    while (!q.empty()) {
        NodeType<EdgeType> u = q.pop();
        if (u == goal) break;
        for (size_t i = 0; i < counts[u]; ++i) {
            size_t eidx = adj[u][i];
            NodeType<EdgeType> v = edges[eidx].dst;
            if (!visited[v]) {
                visited[v] = true;
                prev[v] = static_cast<int>(u);
                via_edge[v] = static_cast<int>(eidx);
                q.push(v);
            }
        }
    }

    BFSResult<EdgeType, num_nodes - 1> result{};
    if (!visited[goal]) return result;

    // reconstruct path (edges)
    size_t len = 0;
    for (int at = goal; at != start; at = prev[at]) {
        // copy edge
        result.path[len++] = edges[via_edge[at]];
    }

    std::reverse(result.path.begin(), result.path.begin() + len);
    result.length = len;
    result.found = true;
    return result;
}
