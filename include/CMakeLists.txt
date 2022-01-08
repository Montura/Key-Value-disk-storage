cmake_minimum_required(VERSION 3.12)
project(include)

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
        volume.h
        storage.h
        )

set(KEY_VALUE_STORAGE_INCLUDE_DIR .)

add_library(key-value-storage-lib INTERFACE)
target_compile_features(key-value-storage-lib INTERFACE cxx_std_17)
target_include_directories(key-value-storage-lib INTERFACE ${KEY_VALUE_STORAGE_INCLUDE_DIR} )