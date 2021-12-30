cmake_minimum_required(VERSION 3.12)
project(key-value-storage)
enable_language(CXX)

find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Failed to find boost library")
endif()

set(SOURCE_FILES test.cpp)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(key-value-storage ${SOURCE_FILES})
target_compile_definitions(key-value-storage PRIVATE
        BOOST_ALL_NO_LIB
        BOOST_TEST_MODULE=unit-cpp
        USE_BOOST_PREBUILT_STATIC_LIBRARY=false
        )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(key-value-storage PRIVATE
        -std=c++17
        -stdlib=libc++
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_options(key-value-storage PRIVATE
       /std:c++17
       /W3
    )
endif()