#include <iostream>
#include <filesystem>
#include <chrono>


namespace fs = std::filesystem;
using std::cout;
using std::endl;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

#include "btree_impl.h"
#include "btree_node_impl.h"

#ifdef UNIT_TESTS
#if USE_BOOST_PREBUILT_STATIC_LIBRARY
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

#include <boost/test/data/test_case.hpp>
#include <boost/range/iterator_range.hpp>


namespace {
    BOOST_AUTO_TEST_CASE(test_b_tree_init) {
        const int n = 1000;
        using BTreeIntInt = BTreeStore<int, int>;
        bool found_all = false, any_not_found = false, remove_all = false;
        std::string db_prefix = "../db_";
        std::string end = ".txt";

        for (int order = 2; order < 101; ++order) {
            auto db_name = db_prefix + std::to_string(order) + end;
            {
                BTreeIntInt btree(db_name, order);
                for (int i = 0; i < n; ++i) {
                    btree.set(i, 65 + i);
                }
            }

            {
                BTreeIntInt btree(db_name, order);
                for (int i = 0; i < n; ++i) {
                    int value = btree.getValue(i);
                    assert(value == i + 65);
                }
            }

            {
                BTreeIntInt btree(db_name, order);
                for (int i = 0; i < n; ++i) {
                    btree.set(i, i + 1);
                }
            }

            {
                BTreeIntInt btree(db_name, order);
                for (int i = 0; i < n; ++i) {
                    int value = btree.getValue(i);
                    assert(value == i + 1);
                }
            }

            {
                BTreeIntInt btree(db_name, order);
                int total_found = 0;
                for (int i = 0; i < n; ++i) {
                    total_found += btree.exist(i);
                }
                found_all = (total_found == n);
            }

            {
                BTreeIntInt btree(db_name, order);
                int total_not_found = 0;
                const int key_shift = 1000;
                for (int i = 0; i < n; ++i) {
                    total_not_found += !btree.exist(key_shift + i);
                }
                any_not_found = (total_not_found == n);
            }

            {
                BTreeIntInt btree(db_name, order);
                int total_deleted = 0;
                for (int i = 0; i < n * 2; ++i) {
                    total_deleted += btree.remove(i);
                }
                remove_all = (total_deleted == n);
            }

            std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
            BOOST_REQUIRE_MESSAGE(found_all && any_not_found && remove_all, msg);
        }
    }
}
#else

void testBuildBTreeStore() {
    {
      BTreeStore<int, int> bTree5("../a.txt", 2);

      for (int i = 0; i < 1000; i++) {
        bTree5.set(i, 65 + i);
      }
      //
      //    cout << "Traversal of the constucted tree is " << endl;
      //    cout << "\n----------------" << endl;
      //    bTreeStore->traverse();
    }
    cout << "\n----------------" << endl;

}


void testFunctionExist() {
    auto bTreeStore = new BTreeStore<int, int>("../a.txt", 2);
    cout << "---------------Test Exist-------------------" << endl;

    int key;
    int value;
    int count = 0;
    int n = 1000;
    for (int i = 0; i < n; ++i) {
        key = i;
        if (bTreeStore->exist(key)) {
            ++count;
//            cout << "Key is exist: " << key << ", ";
//            bTreeStore->get(key, value);
//            cout << "[value]: " << value << endl;
        } else {
            cout << "Key " << key << " is not exist" << endl;
        }
    }
    assert(n == count);
    cout << "---------------------------------------------" << endl;
    delete bTreeStore;
}

void at_exit_handler();

int main() {
    const int handler = std::atexit(at_exit_handler);
    const int n = 10000;
    using BTreeIntInt = BTreeStore<int, int>;
    bool found_all = false, any_not_found = false, remove_all = false;
    std::string db_prefix = "../db_";
    std::string end = ".txt";

    for (int order = 2; order < 101; ++order) {
        auto t1 = high_resolution_clock::now();

        auto db_name = db_prefix + std::to_string(order) + end;
        {
            BTreeIntInt btree(db_name, order);
            for (int i = 0; i < n; ++i) {
                btree.set(i, 65 + i);
            }
        }

        {
            BTreeIntInt btree(db_name, order);
            for (int i = 0; i < n; ++i) {
                int value = btree.getValue(i);
                assert(value == i + 65);
            }
        }

        {
            BTreeIntInt btree(db_name, order);
            for (int i = 0; i < n; ++i) {
                btree.set(i, i + 1);
            }
        }

        {
            BTreeIntInt btree(db_name, order);
            for (int i = 0; i < n; ++i) {
                int value = btree.getValue(i);
                assert(value == i + 1);
            }
        }

        {
            BTreeIntInt btree(db_name, order);
            int total_found = 0;
            for (int i = 0; i < n; ++i) {
                total_found += btree.exist(i);
            }
            found_all = (total_found == n);
        }

        {
            BTreeIntInt btree(db_name, order);
            int total_not_found = 0;
            const int key_shift = n + 1000;
            for (int i = 0; i < n; ++i) {
                total_not_found += !btree.exist(key_shift + i);
            }
            any_not_found = (total_not_found == n);
        }

        {
            BTreeIntInt btree(db_name, order);
            int total_deleted = 0;
            for (int i = 0; i < n * 2; ++i) {
                total_deleted += btree.remove(i);
            }
            remove_all = (total_deleted == n);
        }

        auto t2 = high_resolution_clock::now();

        /* Getting number of milliseconds as a double. */
        duration<double, std::milli> ms_double = t2 - t1;

//        std::cout << ms_int.count() << "ms\n";
//        std::cout <<  << "ms\n";

        std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
        cout << "Passed: " << msg << " in " << ms_double.count() << "ms" <<  endl;
        assert( found_all && any_not_found && remove_all);
    }
    for (int order = 2; order < 101; ++order) {
        auto db_name = db_prefix + std::to_string(order) + end;
        fs::remove(db_name);
    }
}
#endif // UNIT_TESTS