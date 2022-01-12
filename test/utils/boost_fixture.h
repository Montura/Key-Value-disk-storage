#pragma once

#include <filesystem>
#include <iostream>

namespace tests {
    namespace fs = std::filesystem;

    struct MyFixture {
        MyFixture (const std::string path) {
            const std::filesystem::path& p = fs::current_path();
            fs::current_path(p);
            fs::create_directories(path);

#if DEBUG
            BOOST_TEST_MESSAGE( "Setup key_value_op_tests test suite" );
#endif
            try {
                std::uintmax_t count = 0;
                for (auto& de : std::filesystem::directory_iterator(path)) {
                    count += std::filesystem::remove_all(de.path());
                }
#if DEBUG
                std::cout << "deleted " << count << " old files" << std::endl;
#endif
            } catch(const std::exception& ex) {
                std::cout << ex.what() << std::endl;
            }
        }
//        ~MyFixture() { BOOST_TEST_MESSAGE( "teardown fixture" ); }
    };
}