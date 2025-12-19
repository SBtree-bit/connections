cmake_minimum_required(VERSION 4.1)
project(connections)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "include/*.h")

add_library(connections STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(connections PUBLIC include)

add_subdirectory(test)