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

struct TestStat {
    int total_added = 0;
    int total_found = 0;
    int total_not_found = 0;
    int total_removed = 0;
    int total_after_remove = 0;
    int total_after_reopen = 0;

    bool found_all() const {
        return total_added == total_found;
    }

    bool any_not_found() const {
        return total_not_found == 0;
    }

    bool check_deleted(int expected) const {
        return total_removed == expected;
    }

    bool found_all_remained() const {
        return total_after_remove == (total_added -  total_removed);
    }
};

void test() {
    const int n = 10000;
    using BTreeIntInt = BTreeStore<int, int>;
    std::string db_name = "../db_";
    std::string end = ".txt";

    for (int order = 2; order < 101; ++order) {
        TestStat stat;
        {
            BTreeIntInt btree(db_name + std::to_string(order) + end, order);
            for (int i = n - 1; i >= 0; --i) {
                btree.set(i, 65 + i);
                ++stat.total_added;
            }

            for (int i = 0; i < n; ++i) {
                stat.total_found += btree.exist(i);
            }
            assert(stat.found_all());

            const int key_shift = n;
            for (int i = 0; i < n; ++i) {
                stat.total_not_found += btree.exist(key_shift + i);
            }
            assert(stat.any_not_found());

            for (int i = n - 1; i >= 0; i -= 4) {
                stat.total_removed += btree.remove(i);
            }

            for (int i = 0; i < n; ++i) {
                stat.total_after_remove += btree.exist(i);
            }
            assert(stat.found_all_remained());
        }
        std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
        cout << "Passed " + msg << ": " <<
             "\t added: " << stat.total_added << ", " <<
             "found: " << stat.total_found << ", " <<
             "removed: " << stat.total_removed << ", " <<
             "total_after_remove: " << stat.total_after_remove << " \n" ;
        {
            BTreeIntInt btree(db_name + std::to_string(order) + end, order);
            for (int i = 0; i < n; ++i) {
                stat.total_after_reopen += btree.exist(i);
            }
            assert(stat.found_all_remained());
        }
        msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
    }
}

void at_exit_handler();

int main() {
    test();
    return 0;
}
#endif // UNIT_TESTS