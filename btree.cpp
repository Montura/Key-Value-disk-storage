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

using std::cout;
using std::endl;

#include <ctime>
#include <map>
#include <cassert>

#include "test_util.h"

 void test() {
    const int n = 100000;
    std::string db_prefix = "../db_";
    std::string end = ".txt";

    for (int i = 0; i < 3; ++i) {
        auto keys_to_remove = generate_rand_keys();
        for (int order = 2; order < 101; ++order) {
            auto db_name = db_prefix + std::to_string(order) + end;
            auto verify_map = test_keys_create_exist<int, int>(db_name, order, n);
            auto total_found = test_values_get(db_name, order, n, verify_map);
            auto [total_removed, total_after_remove] = test_values_remove(db_name, order, n, verify_map, keys_to_remove);
            test_values_after_remove(db_name, order, n, verify_map);

            std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
            cout << "Passed " + msg << ": " <<
                 "\t added: " << n <<
                 ", found: " << total_found <<
                 ", removed: " << total_removed <<
                 ", total_after_remove: " << total_after_remove << endl;
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