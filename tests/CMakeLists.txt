enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(mpl_graph_bfs_tests
    mpl_graph_bfs_test.cpp
)
target_link_libraries(mpl_graph_bfs_tests
    PRIVATE
    mpl_graph_bfs
    GTest::gtest_main
)

add_executable(consteval_graph_bfs_tests
    consteval_graph_bfs_test.cpp
)
target_link_libraries(consteval_graph_bfs_tests
    PRIVATE
    consteval_graph_bfs
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mpl_graph_bfs_tests)
gtest_discover_tests(consteval_graph_bfs_tests)
