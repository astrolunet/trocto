cmake_minimum_required(VERSION 3.20)
project(trocto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(trocto
    src/lexer.cpp
    src/encoder.cpp
    src/regol.cpp
    src/parser.cpp
    src/lower.cpp
    src/trocto.cpp
    apps/trocto/main.cpp
)

target_include_directories(trocto PRIVATE include src)

install(TARGETS trocto RUNTIME DESTINATION bin)
