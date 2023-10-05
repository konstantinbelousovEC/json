# @copyright Copyright (c) 2023. Created by Konstantin Belousov.
# All rights reserved.

cmake_minimum_required(VERSION 3.25)
project(json-my-lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

set(CMAKE_CXX_STANDARD 20)

set(SOURCES src/json.cpp src/json-builder.cpp)
set(HEADERS include/json.h include/json-builder.h)
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/json-my-lib")
set(INCLUDE_DIR "${OUTPUT_DIR}/include")
set(DEBUG_LIB_DIR "${OUTPUT_DIR}/lib/debug")
set(RELEASE_LIB_DIR "${OUTPUT_DIR}/lib/release")

foreach(HEADER ${HEADERS})
    configure_file(${HEADER} ${INCLUDE_DIR} COPYONLY)
endforeach()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
add_library(json-my-lib-debug STATIC ${SOURCES})
set_target_properties(json-my-lib-debug PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${DEBUG_LIB_DIR}")
set_target_properties(json-my-lib-debug PROPERTIES PREFIX "")
set_target_properties(json-my-lib-debug PROPERTIES OUTPUT_NAME "libjson-my-lib")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
add_library(json-my-lib-release STATIC ${SOURCES})
set_target_properties(json-my-lib-release PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_LIB_DIR}")
set_target_properties(json-my-lib-release PROPERTIES PREFIX "")
set_target_properties(json-my-lib-release PROPERTIES OUTPUT_NAME "libjson-my-lib")

add_executable(test-executable main.cpp include/json.h include/json-builder.h src/json.cpp src/json-builder.cpp tests/log-duration.h tests/test-runner.h tests/tests.h tests/tests.cpp)
target_link_libraries(test-executable json-my-lib-debug)
