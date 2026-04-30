cmake_minimum_required(VERSION 3.16)
project(xmon)

set(CMAKE_CXX_STANDARD 20)

add_executable(xmon
    src/main.cpp
)

target_link_libraries(xmon ncurses)
