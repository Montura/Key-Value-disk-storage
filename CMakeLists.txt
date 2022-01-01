cmake_minimum_required(VERSION 3.12)
project(key-value-storage LANGUAGES CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(Boost_USE_STATIC_LIBS ON)
endif()

message(${CMAKE_CXX_COMPILER})
message(${CMAKE_CXX_COMPILER_VERSION})

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS iostreams REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Failed to find boost library")
endif()

include_directories(${Boost_INCLUDE_DIRS})
message(${Boost_IOSTREAMS_LIBRARY})

add_subdirectory(include)
include_directories(${KEY_VALUE_STORAGE_INCLUDE_DIR})
message(key-value-storage-lib found at: ${KEY_VALUE_STORAGE_INCLUDE_DIR})

set(SOURCE_FILES
        test/main.cpp
        #src/mem_util.cpp
    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE key-value-storage-lib ${Boost_IOSTREAMS_LIBRARY})

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