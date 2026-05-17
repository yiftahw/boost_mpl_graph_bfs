#pragma once

/*
    For the example graph

    adjacency list test graph:
    A -> B -> C -\--> D
           \     |--> E
            \    \--> F
             \-----/
    G
*/

/*
// vertices
struct A{}; struct B{}; struct C{}; struct D{}; struct E{}; struct F{}; struct G{};

// edges
struct A_B{}; struct B_C{}; struct C_D{}; struct C_E{}; struct C_F{}; struct B_F{};

// define the graph structure
typedef mpl::vector<
                mpl::vector<A_B,A,B>,
                mpl::vector<B_C,B,C>,
                mpl::vector<C_D,C,D>,
                mpl::vector<C_E,C,E>,
                mpl::vector<C_F,C,F>,
                mpl::vector<B_F,B,F>
            > my_graph_data_type;
typedef mpl_graph::incidence_list_graph<my_graph_data_type> my_graph_type;

// best route from A to F is A -> B -> F and NOT A -> B -> C -> F
typedef mpl::first<bfs_route_finder<my_graph_type, A, F>::type>::type route_A_to_F;
BOOST_MPL_ASSERT((boost::is_same<bfs_route_found_t<route_A_to_F>, mpl::true_>));
BOOST_MPL_ASSERT((boost::is_same<bfs_route_path_t<route_A_to_F>, mpl::vector3<A, B, F>>));

// there is no route from A to G
typedef mpl::first<bfs_route_finder<my_graph_type, A, G>::type>::type route_A_to_G;
BOOST_MPL_ASSERT((boost::is_same<bfs_route_found_t<route_A_to_G>, mpl::false_>));
*/

#include <boost/msm/mpl_graph/incidence_list_graph.hpp>
#include <boost/msm/mpl_graph/breadth_first_search.hpp>

namespace mpl_graph = boost::msm::mpl_graph;
namespace mpl = boost::mpl;

/**
 * @brief Metafunction to backtrack the path from a target vertex to the source vertex using a parent map.
 * 
 * Given a target vertex and a parent map that associates each vertex with its parent,
 * this metafunction recursively constructs the path from the target vertex back to the source vertex.
 * The path is represented as an MPL vector.
 */

// Recursive case
template <typename Vertex, typename ParentMap>
struct backtrack
{
    using type = typename mpl::push_back<
        typename backtrack<typename mpl::at<ParentMap, Vertex>::type, ParentMap>::type,
        Vertex
    >::type;
};

// Base case: when there is no parent (i.e., mpl::void_)
template <typename ParentMap>
struct backtrack<mpl::void_, ParentMap>
{
    using type = mpl::vector<>;
};


// visitor strategy to build the path on traversal
template<typename TargetNode>
struct route_finder_visitor : mpl_graph::bfs_default_visitor_operations {
    using StateType = 
        mpl::vector4<
            mpl::vector<>, // route
            mpl::map<>,    // vertex to parent mapping for backtracking
            TargetNode,    // vertex we are seeking
            mpl::false_    // did we find our target?
        >;

    template <typename Edge, typename Graph>
    using src_t = typename mpl_graph::source<Edge, Graph>::type;

    template <typename Edge, typename Graph>
    using dst_t = typename mpl_graph::target<Edge, Graph>::type;

    template<typename Edge, typename Graph, typename ParentMap>
    struct insert_parent_t : mpl::insert<
        ParentMap, 
        mpl::pair<dst_t<Edge, Graph>, src_t<Edge, Graph>>
    >::type {};

    // tree edge (i.e an edge that connects to a vertex we didn't discover yet)
    // it means that this edge's source vertex is the fastest way to get to it's destination vertex
    // mark it as it's parent.
    template<typename Edge, typename Graph, typename State>
    struct tree_edge : mpl::if_<
        typename mpl::at_c<State, 3>::type,
        // if already found - do nothing
        State,
        // else - update parent map
        mpl::vector4<
            typename mpl::at_c<State, 0>::type, // route isn't changed here
            typename insert_parent_t<Edge, Graph, typename mpl::at_c<State,1>::type>::type, // update parent map
            typename mpl::at_c<State, 2>::type, // target node unchanged
            typename mpl::at_c<State, 3>::type  // found flag unchanged
        >
    > {};

    // first time we encounter a new vertex
    // if it's the target vertex - it means we found the fastest route to it
    template<typename Vertex, typename Graph, typename State>
    struct discover_vertex : mpl::if_<
        boost::is_same<Vertex, typename mpl::at_c<State,2>::type>,
        // if found - set found flag to true and push to path stack
        mpl::vector4<
            typename backtrack<Vertex, typename mpl::at_c<State,1>::type>::type, // backtrack to build the route
            typename mpl::at_c<State,1>::type, // parent mapping unchanged
            typename mpl::at_c<State,2>::type,  // target node unchanged
            mpl::true_              // set found flag to true
        >,
        // if not found - do nothing
        State
    > {};
};

/**
 * @brief A compile-time breadth-first search route finder that finds paths between nodes in a graph.
 * 
 * This template class specializes the mpl_graph::breadth_first_search algorithm to find
 * routes from a source node to a target node within a given graph structure. It uses
 * a route_finder_visitor to track the search progress and maintain state during traversal.
 * 
 * @tparam Graph The graph type to search within
 * @tparam SourceNode The starting node for the route search
 * @tparam TargetNode The destination node for the route search
 */
template <typename Graph, typename SourceNode, typename TargetNode>
struct bfs_route_finder : mpl_graph::breadth_first_search<
    Graph,
    route_finder_visitor<TargetNode>,
    typename route_finder_visitor<TargetNode>::StateType,
    SourceNode
> {};

template <typename Graph, typename SourceNode, typename TargetNode>
using bfs_route_query_result_t = typename mpl::first<typename bfs_route_finder<Graph, SourceNode, TargetNode>::type>::type;

// // Helper type to extract whether a route was found
// // TODO: add a concept on the type of QueryResult
template <typename QueryResult>
using bfs_route_found_t = typename mpl::at_c<QueryResult, 3>::type;

// // Extract the value of whether a route was found
template <typename QueryResult>
constexpr auto bfs_route_found_v = bfs_route_found_t<QueryResult>::value;

// // Helper type to extract the route path
template <typename QueryResult>
using bfs_route_path_t = typename mpl::at_c<QueryResult, 0>::type;

template <typename QueryResult>
constexpr auto bfs_route_length_v = mpl::size<bfs_route_path_t<QueryResult>>::value;

template <typename QueryResult>
using bfs_parent_map_t = typename mpl::at_c<QueryResult, 1>::type;
