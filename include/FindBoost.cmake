# Find BOOST library and its components
find_package(Boost COMPONENTS iostreams thread REQUIRED)
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Failed to find boost library")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(Boost_USE_STATIC_LIBS ON)
endif()

message(${Boost_IOSTREAMS_LIBRARY})
message(${Boost_THREAD_LIBRARY})

include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${Boost_IOSTREAMS_LIBRARY}
        ${Boost_THREAD_LIBRARY})

target_compile_definitions(${PROJECT_NAME} PRIVATE
        BOOST_ALL_NO_LIB
        BOOST_TEST_MODULE=unit-cpp
        USE_BOOST_PREBUILT_STATIC_LIBRARY=false
        )