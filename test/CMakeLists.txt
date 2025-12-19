file(GLOB_RECURSE TEST_SOURCE_FILES "src/*.cpp")

include_directories(include)



add_executable(connections_test ${TEST_SOURCE_FILES})
target_link_libraries(connections_test PUBLIC connections)