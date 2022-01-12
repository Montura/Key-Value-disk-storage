cmake_minimum_required(VERSION 3.12)
project(key-value-storage-test)
enable_language(CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "valid configurations" FORCE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include("PlatformInfo.cmake")
include_directories(.)

set(SOURCE_FILES
        test_runner/test_runner.h
        test_runner/test_value_generator.h
        utils/boost_fixture.h
        utils/size_info.h
        utils/test_stat.h
        utils/thread_pool.h
        mapped_file_tests.h
        key_value_operations_tests.h
        volume_tests.h
        test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include("${CMAKE_CURRENT_SOURCE_DIR}/../include/FindBoost.cmake")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNIT_TESTS
#    MEM_CHECK
#    DEBUG
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  target_compile_options(${PROJECT_NAME} PRIVATE
      -std=c++17
      -stdlib=libc++
      -Wall
      -Wextra
      -Wno-unused-parameter
      -O3
      )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_definitions(${PROJECT_NAME} PRIVATE
      _WIN32_WINDOWS
      _WINSOCK_DEPRECATED_NO_WARNINGS
      )
  target_compile_options(${PROJECT_NAME} PRIVATE
      /std:c++17
      /W3
      /bigobj
      "$<$<CONFIG:Release>:/GL>"
      "$<$<CONFIG:Release>:/Ox>"
      "$<$<CONFIG:Release>:/Ob2>"
      "$<$<CONFIG:Release>:/Ot>"
      "$<$<CONFIG:Release>:/Oi>"
      "$<$<CONFIG:Release>:/Oy->"
      )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} PRIVATE
      -std=c++17
      -Wall
      -Wextra
      -Wno-unused-parameter
      -O3
      )
endif()