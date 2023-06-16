cmake_minimum_required(VERSION 3.25)
project(json)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_CPP ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB SOURCE_HPP ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB TESTS ${CMAKE_SOURCE_DIR}/tests/*)

add_executable(json main.cpp ${SOURCE_CPP} ${SOURCE_HPP} ${TESTS})
