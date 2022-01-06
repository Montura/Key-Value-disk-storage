cmake_minimum_required(VERSION 3.12)
project(key-value-storage-lib)

include_directories(.)

set(SOURCE_FILES
        btree_impl/btree.h
        btree_impl/btree_impl.h
        btree_impl/btree_node.h
        btree_impl/btree_node_impl.h
        entry.h
        io/file_mapping.h
        io/file_mapping_impl.h
        io/io_manager.h
        io/io_manager_impl.h
        utils/boost_include.h
        utils/forward_decl.h.h
        utils/utils.h
        storage.h
        )

set(KEY_VALUE_STORAGE_INCLUDE_DIR .)

add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
target_include_directories(${PROJECT_NAME} INTERFACE ${KEY_VALUE_STORAGE_INCLUDE_DIR} )