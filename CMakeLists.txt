cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(compile_time_graph_bfs VERSION 1.0.0 LANGUAGES CXX)

find_package(Boost CONFIG REQUIRED)

add_library(consteval_graph_bfs INTERFACE)
target_include_directories(consteval_graph_bfs INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/consteval_graph_bfs/include
    $<INSTALL_INTERFACE:include>
)

add_library(mpl_graph_bfs INTERFACE)
target_include_directories(mpl_graph_bfs INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/mpl_graph_bfs/include
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(mpl_graph_bfs INTERFACE Boost::headers)

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
