cmake_minimum_required(VERSION 3.12)
project(key-value-storage)
enable_language(CXX)

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS iostreams REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Failed to find boost library")
endif()

set(SOURCE_FILES boost_mapped_file.cpp)
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
endif()

target_link_libraries(key-value-storage PRIVATE
    ${Boost_IOSTREAMS_LIBRARY}
)