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
        std::string db_name = "../db_";
        std::string end = ".txt";

        for (int order = 2; order < 101; ++order) {
            BTreeIntInt btree(db_name + std::to_string(order) + end, order);
            for (int i = 0; i < n; ++i) {
                btree.set(i, 65 + i);
            }

            int total_found = 0;
            for (int i = 0; i < n; ++i) {
                total_found += btree.exist(i);
            }
            found_all = (total_found == n);

            int total_not_found = 0;
            const int key_shift = 1000;
            for (int i = 0; i < n; ++i) {
                total_not_found += !btree.exist(key_shift + i);
            }
            any_not_found = (total_not_found == n);

            int total_deleted = 0;
            for (int i = 0; i < n * 2; ++i) {
                total_deleted += btree.remove(i);
            }
            remove_all = (total_deleted == n);

            std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
            BOOST_REQUIRE_MESSAGE(found_all && any_not_found && remove_all, msg);
        }
    }
}
#else

void testBuildBTreeStore() {
    BTreeStore<int, int> bTree5;

    for (int i = 0; i < 50; i++) {
        bTree5.set(i,65 + i);
    }
    //
    //    cout << "Traversal of the constucted tree is " << endl;
    //    cout << "\n----------------" << endl;
    //    bTreeStore->traverse();
    //    cout << "\n----------------" << endl;
    bTree5;
}


void testFunctionExist() {
    auto bTreeStore = new BTreeStore<int, int>();
    cout << "---------------Test Exist-------------------" << endl;

    int key;
    int value;
    for (int i = 0; i < 50; ++i) {
        key = i;
        if (bTreeStore->exist(key)) {
            cout << "Key is exist: " << key << ", ";
//            bTreeStore->get(key, value);
//            cout << "[value]: " << value << endl;
        } else {
            cout << "Key is not exist" << endl;
        }
    }
    cout << "---------------------------------------------" << endl;

}

void at_exit_handler();

int main() {
    const int handler = std::atexit(at_exit_handler);


    testBuildBTreeStore();
    testFunctionExist();

    return 0;
}
#endif // UNIT_TESTS