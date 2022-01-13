#pragma once

#include <filesystem>
#include <iostream>

#include "utils/boost_include.h"
#include "utils/mem_util.h"

namespace tests {
    namespace fs = std::filesystem;

    struct MyFixture {
        const std::string path;

        explicit MyFixture(const std::string& path) : path(path) {
            BOOST_TEST_MESSAGE("Enter " + path + " test suite");
#ifdef MEM_CHECK
            atexit(at_exit_handler);
#endif
            const std::filesystem::path& p = fs::current_path();
            fs::current_path(p);
            fs::create_directories(path);

            try {
                for (auto& dir_entry: std::filesystem::directory_iterator(path))
                    std::filesystem::remove_all(dir_entry.path());
            } catch (const std::exception& ex) {
                std::cout << ex.what() << std::endl;
            }
        }
        ~MyFixture() {
            BOOST_TEST_MESSAGE("Leave " + path + " test suite");
#ifdef MEM_CHECK
            BOOST_TEST_MESSAGE( "Mem stat for " + path + " test suite:");
#endif
#ifndef DEBUG
            fs::remove_all(path);
#endif
        }
    };

    namespace b_decorator = boost::unit_test::decorator;
    using FixtureT = b_decorator::fixture_t(*)(const std::string& path);
    FixtureT CleanBeforeTest = boost::unit_test::fixture<MyFixture>;
}