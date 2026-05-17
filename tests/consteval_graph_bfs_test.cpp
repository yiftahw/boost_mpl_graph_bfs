#include <gtest/gtest.h>

#include "consteval_graph_bfs/consteval_graph_shortest_path.hpp"

enum Node { A, B, C, D, E, F, G };
static constexpr size_t node_count = G + 1;

struct Edge {
    Node src;
    Node dst;
    constexpr bool operator==(const Edge& other) const = default;
};
static_assert(EdgeConcept<Edge>);

constexpr std::array<Edge, 6> edges = {{
    {A, B},
    {B, C},
    {C, D},
    {C, E},
    {C, F},
    {B, F}
}};

// ---------- compile time test ----------

TEST(ConstexprBFS, RouteAtoF) {
    constexpr auto route_A_to_F = bfs_find_shortest_path<edges, node_count, A, F>();
    static_assert(std::is_same_v<decltype(route_A_to_F), const std::array<Edge, 2>>);
    static_assert(route_A_to_F.size() == 2);
    static_assert(route_A_to_F[0] == Edge{A, B});
    static_assert(route_A_to_F[1] == Edge{B, F});
}

TEST(ConstexprBFS, NoRouteAtoG) {
    constexpr auto route_A_to_G = bfs_find_shortest_path<edges, node_count, A, G>();
    static_assert(std::is_same_v<decltype(route_A_to_G), const std::array<Edge, 0>>);
    static_assert(route_A_to_G.size() == 0);
}
