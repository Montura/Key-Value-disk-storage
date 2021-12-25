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
    BTreeStore<int, int> bTree5("../a.txt", 2);

    for (int i = 0; i < 1000; i++) {
        bTree5.set(i, 65 + i);
    }
    //
    //    cout << "Traversal of the constucted tree is " << endl;
    //    cout << "\n----------------" << endl;
    //    bTreeStore->traverse();
    //    cout << "\n----------------" << endl;
    bTree5;
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

}

//void at_exit_handler();

int main() {
//    const int handler = std::atexit(at_exit_handler);


//    testBuildBTreeStore();
    testFunctionExist();

    return 0;
}
#endif // UNIT_TESTS