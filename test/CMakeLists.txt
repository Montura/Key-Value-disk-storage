cmake_minimum_required(VERSION 3.12)
project(test)
enable_language(CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(Boost_USE_STATIC_LIBS ON)
endif()

message(${CMAKE_CXX_COMPILER})
message(${CMAKE_CXX_COMPILER_VERSION})

find_package(Boost COMPONENTS iostreams thread REQUIRED)
if (NOT Boost_FOUND)
  message(FATAL_ERROR "Failed to find boost library")
endif()

include_directories(../include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${KEY_VALUE_STORAGE_INCLUDE_DIR})

message(${Boost_IOSTREAMS_LIBRARY})
message(${Boost_THREAD_LIBRARY})
message(key-value-storage-lib found at: ${KEY_VALUE_STORAGE_INCLUDE_DIR})

set(SOURCE_FILES test.cpp)

add_executable(key-value-storage-test ${SOURCE_FILES})

target_link_libraries(key-value-storage-test PRIVATE
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_THREAD_LIBRARY})

target_compile_definitions(key-value-storage-test PRIVATE
    BOOST_ALL_NO_LIB
    BOOST_TEST_MODULE=unit-cpp
    USE_BOOST_PREBUILT_STATIC_LIBRARY=false
    UNIT_TESTS
#    MEM_CHECK
    )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  target_compile_options(key-value-storage-test PRIVATE
      -std=c++17
      -stdlib=libc++
      -Wall
      -Wextra
      -Wno-unused-parameter
      -O3
      )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_options(key-value-storage-test PRIVATE
      /std:c++17
      /W3
      /GL
      #       /Ox
      /Ob2
      /Oi
      /Oy-
      )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(key-value-storage-test PRIVATE
      -std=c++17
      -Wall
      -Wextra
      -Wno-unused-parameter
      -O3
      )
endif()