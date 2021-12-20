#include <iostream>

using std::cout;
using std::endl;

#include "btree_impl.h"
#include "btree_node_impl.h"

void testBTree() {
    BTree<int, std::string, 3> treeString;
    BTree<int, char*, 10> treeBlob;
//    cout << sizeof(std::unique_ptr<Node<int, int>>) << endl;
//    cout << sizeof(BTreeNode<int, int>*) << endl;
//    cout << sizeof(std::unique_ptr<int>) << endl;

    auto bTree = std::make_unique<BTree<int, int, 5>>();

    for (int i = 0; i < 50; i += 2) {
        bTree->set(i, char (65 + i)); // char for view in debugger
    }
    for (int i = 1; i < 50; i += 2) {
        bTree->set(i, char (65 + i)); // char for view in debugger
    }

    for (int i = 1; i < 50; i += 2) {
        bTree->set(i, -165);
    }

    int found = 0;
    for (int i = 0; i < 50; ++i) {
        bool hasKey = bTree->exist(i);
        if (!hasKey) {
            cout << "Can't find key: " << i << endl;
        }
        found += hasKey ? 1 : 0;
    }
    cout << "Total keys found: " << found << endl;

    cout << endl;
    cout << "Tree traversal" << endl;
    cout << "-----------------------------" << endl;
    bTree->traverse();
    cout << "-----------------------------" << endl;
}

