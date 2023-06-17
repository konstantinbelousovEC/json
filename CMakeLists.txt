#cmake_minimum_required(VERSION 3.25)
#project(json)
#
#set(CMAKE_CXX_STANDARD 17)
#
#file(GLOB SOURCE_CPP ${CMAKE_SOURCE_DIR}/src/*.cpp)
#file(GLOB SOURCE_HPP ${CMAKE_SOURCE_DIR}/include/*.h)
#file(GLOB TESTS ${CMAKE_SOURCE_DIR}/tests/*)
#
#add_executable(json main.cpp ${SOURCE_CPP} ${SOURCE_HPP} ${TESTS})

cmake_minimum_required(VERSION 3.25)
project(json-my-lib)

set(CMAKE_CXX_STANDARD 17)

# Пути к исходным файлам
set(SOURCES src/json.cpp src/json-builder.cpp)

# Пути к заголовочным файлам
set(HEADERS include/json.h include/json-builder.h)

# Путь к директории собранной библиотеки
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/json-my-lib")

# Путь к директории заголовочных файлов
set(INCLUDE_DIR "${OUTPUT_DIR}/include")

# Путь к директории сборки debug-библиотеки
set(DEBUG_LIB_DIR "${OUTPUT_DIR}/lib/debug")

# Путь к директории сборки release-библиотеки
set(RELEASE_LIB_DIR "${OUTPUT_DIR}/lib/release")

# Копирование заголовочных файлов
foreach(HEADER ${HEADERS})
    configure_file(${HEADER} ${INCLUDE_DIR} COPYONLY)
endforeach()

# Debug-конфигурация
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
add_library(json-my-lib-debug STATIC ${SOURCES})
set_target_properties(json-my-lib-debug PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${DEBUG_LIB_DIR}")
set_target_properties(json-my-lib-debug PROPERTIES PREFIX "")
set_target_properties(json-my-lib-debug PROPERTIES OUTPUT_NAME "libjson-my-lib")

# Release-конфигурация
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
add_library(json-my-lib-release STATIC ${SOURCES})
set_target_properties(json-my-lib-release PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_LIB_DIR}")
set_target_properties(json-my-lib-release PROPERTIES PREFIX "")
set_target_properties(json-my-lib-release PROPERTIES OUTPUT_NAME "libjson-my-lib")

# Добавление исполняемого файла для сборки проекта
add_executable(test-executable main.cpp tests/log-duration.h tests/test-runner.h tests/tests.h tests/tests.cpp)
target_link_libraries(test-executable json-my-lib-debug)  # Используем debug-библиотеку
