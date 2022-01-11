cmake_minimum_required(VERSION 3.12)
project(test)
enable_language(CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "valid configurations" FORCE)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(Boost_USE_STATIC_LIBS ON)
endif()

message("Platform = ${CMAKE_SYSTEM_NAME} v${CMAKE_SYSTEM_VERSION}")
message("Compiler = ${CMAKE_CXX_COMPILER}")
message("  ${CMAKE_CXX_COMPILER_ID} v.${CMAKE_CXX_COMPILER_VERSION}")
message("BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  message("  target arch = x86-64, CPU = ${CMAKE_SYSTEM_PROCESSOR}")
  set(TARGET_ARCH x64)
elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
  set(TARGET_ARCH x86-64)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message(FATAL_ERROR MSVC compiler is expected for ${TARGET_ARCH})
  endif()
  message("  target arch = x86, CPU = ${CMAKE_SYSTEM_PROCESSOR}")
endif()


find_package(Boost COMPONENTS iostreams thread REQUIRED)
if (NOT Boost_FOUND)
  message(FATAL_ERROR "Failed to find boost library")
endif()

include_directories(
        .
        ../include
        ${Boost_INCLUDE_DIRS}
        ${KEY_VALUE_STORAGE_INCLUDE_DIR})

message(${Boost_IOSTREAMS_LIBRARY})
message(${Boost_THREAD_LIBRARY})
message(key-value-storage-lib found at: ${KEY_VALUE_STORAGE_INCLUDE_DIR})

set(SOURCE_FILES
        test_runner/test_runner.h
        test_runner/test_value_generator.h
        utils/size_info.h
        utils/test_stat.h
        utils/thread_pool.h
        mapped_file_tests.h
        storage_tests.h
        test.cpp
)

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
#    DEBUG
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
  target_compile_definitions(key-value-storage-test PRIVATE
      _WIN32_WINDOWS
      _WINSOCK_DEPRECATED_NO_WARNINGS
      )
  target_compile_options(key-value-storage-test PRIVATE
      /std:c++17
      /W3
      "$<$<CONFIG:Release>:/GL>"
      "$<$<CONFIG:Release>:/Ox>"
      "$<$<CONFIG:Release>:/Ob2>"
      "$<$<CONFIG:Release>:/Ot>"
      "$<$<CONFIG:Release>:/Oi>"
      "$<$<CONFIG:Release>:/Oy->"
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