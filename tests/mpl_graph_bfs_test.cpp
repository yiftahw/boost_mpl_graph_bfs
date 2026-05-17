#include <type_traits>

#include <gtest/gtest.h>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>

#include "mpl_graph_bfs/boost_mpl_graph_shortest_path.hpp"

// Define the test graph structure from the example in the header
// adjacency list test graph:
// A -> B -> C -\--> D
//        \     |--> E
//         \    \--> F
//          \-----/
// G (disconnected)



// vertices
struct A{};
struct B{};
struct C{};
struct D{};
struct E{};
struct F{};
struct G{};
struct H{};

// edges
struct A_B{};
struct A_C{};
struct B_C{}; 
struct B_D{};
struct B_F{};
struct C_D{}; 
struct C_E{}; 
struct C_F{};
struct D_F{};
struct F_H{};
struct E_G{};

// define the graph structure
typedef mpl::vector<
    mpl::vector<A_B, A, B>,
    mpl::vector<B_C, B, C>,
    mpl::vector<C_D, C, D>,
    mpl::vector<C_E, C, E>,
    mpl::vector<C_F, C, F>,
    mpl::vector<B_F, B, F>
> my_graph_data_type;
typedef mpl_graph::incidence_list_graph<my_graph_data_type> my_graph_type;

#define EDGE(src,dst) mpl::vector<src##_##dst, src, dst>

TEST(BFSRouteFinderTest, BacktrackingMetaFunction) {
    // simple testcase for backtrack
    struct V1{};
    struct V2{};
    struct V3{};
    using parent_map_example = mpl::map<
        mpl::pair<V3, V2>,
        mpl::pair<V2, V1>
    >;
    // check that if vertex not in map, we get mpl::void_
    static_assert(boost::is_same<mpl::at<parent_map_example, V1>::type, mpl::void_>::value, "parent map test failed");

    using backtrack_test_no_parent = typename backtrack<V1, parent_map_example>::type;
    static_assert(mpl::equal<backtrack_test_no_parent, mpl::vector1<V1>>::value, "backtrack test failed");

    using backtrack_test_1_step = typename backtrack<V2, parent_map_example>::type;
    static_assert(mpl::equal<backtrack_test_1_step, mpl::vector2<V1, V2>>::value, "backtrack test failed");

    using backtrack_test = typename backtrack<V3, parent_map_example>::type;
    static_assert(mpl::equal<backtrack_test, mpl::vector3<V1, V2, V3>>::value, "backtrack test failed");
}

// // Test that a route exists from A to F and is the shortest path A -> B -> F
TEST(BFSRouteFinderTest, MplMapParentMapping) {
    using ref_parent_map_t = mpl::map<mpl::pair<B, A>>;
    static_assert(boost::is_same<mpl::at<ref_parent_map_t, B>::type, A>::value, "parent map test failed");
    static_assert(boost::is_same<mpl::at<ref_parent_map_t, C>::type, mpl::void_>::value, "parent map test failed");

    // push to the map
    using updated_parent_map_t = typename mpl::insert<ref_parent_map_t, mpl::pair<C, B>>::type;
    static_assert(boost::is_same<mpl::at<updated_parent_map_t, C>::type, B>::value, "parent map test failed");
    
    // can we still access B -> A?
    static_assert(boost::is_same<mpl::at<updated_parent_map_t, B>::type, A>::value, "parent map test failed");

    // override B -> D
    using overridden_parent_map_t = typename mpl::insert<updated_parent_map_t, mpl::pair<B, D>>::type;
    static_assert(boost::is_same<mpl::at<overridden_parent_map_t, B>::type, D>::value, "parent map test failed");
}

// test mpl_graph::target and mpl_graph::source
TEST(BFSRouteFinderTest, MplGraphSourceTarget) {
    using graph_edges_t = mpl::vector<
       mpl::vector<A_B, A, B>,
       mpl::vector<B_D, B, D>,
       mpl::vector<D_F, D, F>
    >;
    using graph_t = mpl_graph::incidence_list_graph<graph_edges_t>;

    using edge_AB_source = typename mpl_graph::source<A_B, graph_t>::type;
    using edge_AB_target = typename mpl_graph::target<A_B, graph_t>::type;
    static_assert(boost::is_same<edge_AB_source, A>::value, "mpl_graph source test failed");
    static_assert(boost::is_same<edge_AB_target, B>::value, "mpl_graph target test failed");
}

// // no route from A to G
TEST(BFSRouteFinderTest, NoRouteFromAToG) {
    using route_A_to_G = typename bfs_route_query_result_t<my_graph_type, A, G>::type;
    
    using route_found_type = bfs_route_found_t<route_A_to_G>;
    static constexpr bool found = bfs_route_found_v<route_A_to_G>;
    static_assert(!found);

    using route_type = bfs_route_path_t<route_A_to_G>;
    static constexpr auto route_length = bfs_route_length_v<route_A_to_G>;

    using parent_map_type = bfs_parent_map_t<route_A_to_G>;

    // using parent_map = bfs_parent_map_t<route_A_to_G>;
    // using route = bfs_route_path_t<route_A_to_G>;
    // static constexpr auto route_size = bfs_route_length_v<route_A_to_G>;
    // // Check that route was not found
    // static_assert(!bfs_route_found_v<route_A_to_G>);
}

TEST(BFSRouteFinderTest, TestRoute1) {
    using graph_edges_t = mpl::vector<
        EDGE(A,B),
        EDGE(B,D),
        EDGE(D,F)
    >;
    using graph_t = mpl_graph::incidence_list_graph<graph_edges_t>;
    using query_result_t = bfs_route_query_result_t<graph_t, A, F>;

    static_assert(bfs_route_found_v<query_result_t>, "Route from A to F should be found");

    using route_t = bfs_route_path_t<query_result_t>;
    static_assert(mpl::equal<route_t, mpl::vector4<A, B, D, F>>::value, "Route from A to F should be A -> B -> D -> F");

    static constexpr auto route_length = bfs_route_length_v<query_result_t>;
    static_assert(route_length == 4);
}

TEST(BFSRouteFinderTest, Test2) {
    using graph_2_edges_t = mpl::vector<
        EDGE(A,B),
        EDGE(B,D),
        EDGE(D,F),
        EDGE(F,H),
        EDGE(A,C),
        EDGE(C,E),
        EDGE(E,G)
    >;
    using graph_2_t = mpl_graph::incidence_list_graph<graph_2_edges_t>;

    using route_A_to_H = bfs_route_query_result_t<graph_2_t, A, H>;
    static_assert(bfs_route_found_v<route_A_to_H>);
    using route = bfs_route_path_t<route_A_to_H>;
    static_assert(mpl::equal<route, mpl::vector<A,B,D,F,H>>::value);
}
