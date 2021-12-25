cmake_minimum_required(VERSION 3.12)
project(key-value-storage)
enable_language(CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS iostreams serialization REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Failed to find boost library")
endif()

set(SOURCE_FILES
#        boost_mapped_file.cpp
        btree.cpp
        btree.h
        btree_impl.h
        btree_node.h
        btree_node_impl.h
        entry.h
#        mem_util.cpp
        file_mapping.cpp
        file_mapping.h
        )
include_directories(${Boost_INCLUDE_DIRS})

add_executable(key-value-storage ${SOURCE_FILES})
target_compile_definitions(key-value-storage PRIVATE
    BOOST_ALL_NO_LIB
    BOOST_TEST_MODULE=unit-cpp
    USE_BOOST_PREBUILT_STATIC_LIBRARY=false
    UNIT_TESTS
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

message(${Boost_IOSTREAMS_LIBRARY})
message(${Boost_SERIALIZATION_LIBRARY})
target_link_libraries(key-value-storage PRIVATE
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
)