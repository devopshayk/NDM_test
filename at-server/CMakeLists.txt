cmake_minimum_required(VERSION 3.16)
project(at_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(atserver_core
    src/pattern.cpp
    src/rules.cpp
    src/serial.cpp
    src/server.cpp
    src/text.cpp
)
target_include_directories(atserver_core PUBLIC include)

add_executable(at-server src/main.cpp)
target_link_libraries(at-server PRIVATE atserver_core)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(atserver_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(at-server PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(test_pattern tests/test_pattern.cpp)
target_link_libraries(test_pattern PRIVATE atserver_core)
add_test(NAME pattern COMMAND test_pattern)