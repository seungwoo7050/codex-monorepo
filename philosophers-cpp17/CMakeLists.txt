cmake_minimum_required(VERSION 3.16)
project(philosophers-cpp17 VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(philosophers
    src/main.cpp
    src/simulation.cpp
)

target_include_directories(philosophers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
add_test(
    NAME PhilosophersDeadlockDemo
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/deadlock_demo.sh $<TARGET_FILE:philosophers>
)
add_test(
    NAME PhilosophersOrderedStrategy
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/ordered_strategy.sh $<TARGET_FILE:philosophers>
)
add_test(
    NAME PhilosophersWaiterStrategy
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/waiter_strategy.sh $<TARGET_FILE:philosophers>
)
