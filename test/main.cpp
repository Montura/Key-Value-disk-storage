#ifdef UNIT_TESTS
#if USE_BOOST_PREBUILT_STATIC_LIBRARY
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif




namespace {
    BOOST_AUTO_TEST_CASE(test_b_tree_init) {
        const int n = 1000;
        using BTreeIntInt = BTreeStore<int, int>;
        bool found_all = false, any_not_found = false, remove_all = false;
        std::string db_name = "../db_";
        std::string end = ".txt";

        for (int order = 2; order < 101; ++order) {
            int total_deleted = 0;
            {
                BTreeIntInt btree(db_name + std::to_string(order) + end, order);
                for (int i = n - 1; i >= 0; --i) {
                    btree.set(i, 65 + i);
                }

                int total_found = 0;
                for (int i = 0; i < n; ++i) {
                    total_found += btree.exist(i);
                }
                assert(total_found == n);

                int total_not_found = 0;
                const int key_shift = n;
                for (int i = 0; i < n; ++i) {
                    total_not_found += !btree.exist(key_shift + i);
                }
                any_not_found = (total_not_found == n);

                int cycles = 0;
                for (int i = n - 1; i >= 0; i -= 7) {
                    ++cycles;
                    total_deleted += btree.remove(i);
                }
                remove_all = (total_deleted == cycles);

                for (int i = 0; i < n; i += 31) {
                    ++cycles;
                    total_deleted += btree.remove(i);
                }
                remove_all = (total_deleted == cycles);


                total_found = 0;
                for (int i = 0; i < n; ++i) {
                    total_found += btree.exist(i);
                }
                found_all = (total_found == (n -  total_deleted));
            }
            std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
            BOOST_REQUIRE_MESSAGE(found_all && any_not_found && remove_all, msg);

            {
                BTreeIntInt btree(db_name + std::to_string(order) + end, order);
                int total_found = 0;
                for (int i = 0; i < n; ++i) {
                    total_found += btree.exist(i);
                }
                found_all = (total_found == (n - total_deleted));
            }
            msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
            BOOST_REQUIRE_MESSAGE(found_all, msg);

        }
    }
}
#else

#include <iostream>
#include <ctime>
#include <map>
#include <cassert>
#include <chrono>

using std::cout;
using std::endl;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

#include "key.h"
#include "test_util.h"
using namespace btree_test;

void test() {
    const int n = 10000;
    std::string db_prefix = "../db_";
    std::string end = ".txt";

    for (int i = 0; i < 25; ++i) {
        auto keys_to_remove = generate_rand_keys();
        for (int order = 2; order < 51; ++order) {
            auto db_name = db_prefix + std::to_string(order);
            run<int32_t, int32_t>(db_name + "_i32" + end, order, n, keys_to_remove);
            run<int32_t, int64_t>(db_name + "_i64" + end, order, n, keys_to_remove);
            run<int32_t, float>(db_name + "_f" + end, order, n, keys_to_remove);
            run<int32_t, double>(db_name + "_d" + end, order, n, keys_to_remove);
            run<int32_t, std::string>(db_name + "_str" + end, order, n, keys_to_remove);
            run<int32_t, std::wstring>(db_name + "_wtr" + end, order, n, keys_to_remove);
        }
        std::cout << "iter: " << i << endl;
    }
}

void at_exit_handler();

int main() {
    std::srand(std::time(nullptr)); // use current time as seed for random generator
    test();
    return 0;
}


#endif // UNIT_TESTS