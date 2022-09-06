cmake_minimum_required(VERSION 2.6)
 
# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
 
file(GLOB SRC
     "*.h"
     "*.cpp"
)
add_executable(run_tests ${SRC})

add_test(NAME run_tests COMMAND run_tests)