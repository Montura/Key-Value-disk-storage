cmake_minimum_required(VERSION 3.12)
project(key-value-storage)
enable_language(CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS iostreams REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Failed to find boost library")
endif()

include_directories(include/)

set(SOURCE_FILES
        src/btree.cpp
        include/btree.h
        include/btree_impl.h
        include/btree_node_impl.h
        include/entry.h
#        mem_util.cpp
        include/io/file_mapping.h
        include/io/file_mapping_impl.h
#        file_mapping_tests.cpp
        include/io/io_manager.h
        include/io/io_manager_impl.h
        include/utils/test_util.h
        include/utils/utils.h
        )
include_directories(${Boost_INCLUDE_DIRS})

add_executable(key-value-storage ${SOURCE_FILES})
target_compile_definitions(key-value-storage PRIVATE
    BOOST_ALL_NO_LIB
    BOOST_TEST_MODULE=unit-cpp
    USE_BOOST_PREBUILT_STATIC_LIBRARY=false
#    UNIT_TESTS
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(key-value-storage PRIVATE
        -std=c++17
        -stdlib=libc++
        -Wall
        -Wextra
        -Wno-unused-parameter
        -O3
    )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_options(key-value-storage PRIVATE
       /std:c++17
       /W3
       /GL
#       /Ox
       /Ob2
       /Oi
       /Oy-
    )
endif()

message(${Boost_IOSTREAMS_LIBRARY})
target_link_libraries(key-value-storage PRIVATE
    ${Boost_IOSTREAMS_LIBRARY}
)