#include <iostream>

using std::cout;
using std::endl;

#include "btree_impl.h"
#include "btree_node_impl.h"

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

#include <cassert>

void test() {
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


            total_found = 0;
            for (int i = 0; i < n; ++i) {
                total_found += btree.exist(i);
            }
            found_all = (total_found == (n -  total_deleted));
        }
        std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
        assert(found_all && any_not_found && remove_all);

        {
            BTreeIntInt btree(db_name + std::to_string(order) + end, order);
            int total_found = 0;
            for (int i = 0; i < n; ++i) {
                total_found += btree.exist(i);
            }
            found_all = (total_found == (n - total_deleted));
        }
        msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
        assert(found_all);
        cout << "Passed " + msg << endl;

    }
}

void at_exit_handler();

int main() {
    test();
    return 0;
}
#endif // UNIT_TESTS