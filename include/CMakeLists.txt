cmake_minimum_required(VERSION 3.12)
project(key)

include_directories(${Boost_INCLUDE_DIRS} .)

set(SOURCE_FILES
        btree.h
        btree_impl.h
        btree_node_impl.h
        entry.h
        io/file_mapping.h
        io/file_mapping_impl.h
        io/io_manager.h
        io/io_manager_impl.h
        utils/boost_include.h
        ../test/test_util.h
        utils/utils.h
        )

set(HELLO_INCLUDE_DIR .)

add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
target_include_directories(${PROJECT_NAME} INTERFACE ${HELLO_INCLUDE_DIR} )