cmake_minimum_required(VERSION 3.16)
project(webserv-cpp17 VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(webserv
    src/main.cpp
)

enable_testing()
add_test(
    NAME WebservRespondsOnce
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_webserv.sh $<TARGET_FILE:webserv>
)
add_test(
    NAME WebservMultiConnections
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_webserv_multi.sh $<TARGET_FILE:webserv>
)
add_test(
    NAME WebservHostHeader
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_webserv_host_header.sh $<TARGET_FILE:webserv>
)
add_test(
    NAME WebservKeepAlive
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_webserv_keepalive.sh $<TARGET_FILE:webserv>
)
add_test(
    NAME WebservDynamicHandlers
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_webserv_dynamic.sh $<TARGET_FILE:webserv>
)
