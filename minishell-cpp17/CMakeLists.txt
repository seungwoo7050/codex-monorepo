cmake_minimum_required(VERSION 3.16)
project(minishell-cpp17 VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(minishell
    src/main.cpp
)

enable_testing()
add_test(
    NAME MinishellRunsEcho
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_echo.sh $<TARGET_FILE:minishell>
)
add_test(
    NAME MinishellExpandsEnv
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/env_expansion.sh $<TARGET_FILE:minishell>
)
add_test(
    NAME MinishellBuiltinsCdEnv
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/builtin_cd_env.sh $<TARGET_FILE:minishell>
)
add_test(
    NAME MinishellExitBuiltinStatus
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/builtin_exit_status.sh $<TARGET_FILE:minishell>
)
add_test(
    NAME MinishellPipelineBasic
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/pipeline_basic.sh $<TARGET_FILE:minishell>
)
add_test(
    NAME MinishellRedirectionBasic
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/redirection_basic.sh $<TARGET_FILE:minishell>
)
add_test(
    NAME MinishellSignalInterrupt
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/signal_interrupt.sh $<TARGET_FILE:minishell>
)
add_test(
    NAME MinishellEofExit
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/eof_exit.sh $<TARGET_FILE:minishell>
)
