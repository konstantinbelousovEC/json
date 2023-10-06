# @copyright Copyright (c) 2023. Created by Konstantin Belousov.
# All rights reserved.

cmake_minimum_required(VERSION 3.25)
project(json-lib)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES json.cpp json-builder.cpp)
set(HEADER_FILES json.h json-builder.h)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/json-lib")
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/json-lib")

file(MAKE_DIRECTORY "${OUTPUT_DIR}/include")
file(MAKE_DIRECTORY "${OUTPUT_DIR}/lib")
file(MAKE_DIRECTORY "${OUTPUT_DIR}/lib/debug")
file(MAKE_DIRECTORY "${OUTPUT_DIR}/lib/release")

set(INCLUDE_DIR "${OUTPUT_DIR}/include")
set(DEBUG_LIB_DIR "${OUTPUT_DIR}/lib/debug")
set(RELEASE_LIB_DIR "${OUTPUT_DIR}/lib/release")

# Copy headers to include dir of static lib
foreach(HEADER ${HEADER_FILES})
    configure_file(${HEADER} ${INCLUDE_DIR} COPYONLY)
endforeach()

# Debug version
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
add_library(json-debug STATIC ${SOURCE_FILES})
set_target_properties(json-debug PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${DEBUG_LIB_DIR}")
set_target_properties(json-debug PROPERTIES PREFIX "")
set_target_properties(json-debug PROPERTIES OUTPUT_NAME "libjson")

# Release version
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
add_library(json-release STATIC ${SOURCE_FILES})
set_target_properties(json-release PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_LIB_DIR}")
set_target_properties(json-release PROPERTIES PREFIX "")
set_target_properties(json-release PROPERTIES OUTPUT_NAME "libjson")

add_executable(test-executable main.cpp tests/log-duration.h tests/test-runner.h tests/tests.h tests/tests.cpp)
target_link_libraries(test-executable json-debug)
