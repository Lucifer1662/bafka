cmake_minimum_required(VERSION 2.6)
 
enable_testing()

set(CMAKE_CXX_STANDARD 17)

 
file(GLOB SRC
     "*.h"
     "*.cpp"
)
include(GoogleTest)


add_executable(run_tests ${SRC})

target_link_libraries(run_tests bafka)


# SET(CMAKE_INSTALL_PREFIX "bafka" FORCE)
target_link_libraries(run_tests PUBLIC GTest::gtest_main)

# target_link_libraries(run_tests PUBLIC bafka)
# target_compile_definitions(bafka PRIVATE)

# gtest_discover_tests(run_tests)

# add_test(NAME run_tests COMMAND run_tests)