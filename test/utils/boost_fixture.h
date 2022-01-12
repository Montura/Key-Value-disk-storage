#pragma once

#include <filesystem>
#include <iostream>

#include "utils/boost_include.h"

namespace tests {
    namespace fs = std::filesystem;

    struct MyFixture {
        MyFixture (const std::string path) {
            const std::filesystem::path& p = fs::current_path();
            fs::current_path(p);
            fs::create_directories(path);

            try {
                for (auto& dir_entry : std::filesystem::directory_iterator(path))
                    std::filesystem::remove_all(dir_entry.path());
            } catch(const std::exception& ex) {
                std::cout << ex.what() << std::endl;
            }
        }
//        ~MyFixture() { BOOST_TEST_MESSAGE( "teardown fixture" ); }
    };

    namespace b_decorator = boost::unit_test::decorator;
    using FixtureT = b_decorator::fixture_t(*)(const std::string& str);
    FixtureT CleanBeforeTest = boost::unit_test::fixture<MyFixture>;
}