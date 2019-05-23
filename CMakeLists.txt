cmake_minimum_required(VERSION 3.0.2)

project(test-static-dtor LANGUAGES CXX)

option (USE_SHARED "Compile with shared library" ON)

if (USE_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

add_compile_options(-std=c++11 -g -O0 -rdynamic -fPIC)

add_executable(bar bar.cpp)

add_library(foo ${LIB_TYPE} foo.cpp)
target_compile_options(foo PRIVATE -fPIC)

add_library(baz SHARED baz.cpp)
target_link_libraries(baz foo)

add_dependencies(bar foo)
target_link_libraries(bar foo dl)

add_executable(bar_nofoo bar_nofoo.cpp)
target_link_libraries(bar_nofoo dl)
