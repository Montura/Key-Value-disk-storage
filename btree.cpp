#include <vector>
#include <cstdint>
#include <iostream>

using std::cout;
using std::endl;

/** Common Interface */
template <typename Key, typename Value, typename Oit>
struct IKeyValueStorage {
    virtual bool insert(const Key& key, const Value& value) = 0;
    virtual Oit get(const Key& key) = 0;
    virtual bool remove(const Key& key) = 0;

    virtual ~IKeyValueStorage() { };
};

typedef std::istream_iterator<char> IstreamIterator;

/** Node */ //
template <typename K, typename V, int t = 25>
struct BTreeNode {
    static constexpr int max_key_num = 2 * t - 1;
    static constexpr int max_child_num = 2 * t;
    using Node = BTreeNode<K, V, t>;

    uint8_t is_leaf = 0;
    std::vector<K> keys;
    std::vector<BTreeNode*> children;
    std::vector<V*> values; // only in leaves
    int used_keys = 0;

    BTreeNode() = delete;

    explicit BTreeNode(bool is_leaf = false):
        is_leaf(is_leaf),
        keys(max_key_num, -1),
        children(max_child_num),
        values(max_key_num)
    {}

    ~BTreeNode() {
        if (!is_leaf) {
            for (int i = 0; i < used_keys + 1; ++i) {
                delete children[i];
            }
        } else {
            for (int i = 0; i < used_keys; ++i)
                delete values[i];
        }
    }

    int key_pos_in_leaf_node(const K& key) const {
        assert(is_leaf);
        for (int i = 0; i < used_keys; ++i) {
            if (keys[i] == key)
                return i;
        }
        return used_keys;
    }

    bool has_key(const K& key) const {
        return key_pos_in_leaf_node(key) != used_keys;
    }

    int find_key_pos(const K& key) const {
        int pos = 0;
        while (pos < used_keys && keys[pos] < key)
            ++pos;
        return pos;
    }

    void set(int pos, const V& value) {
        delete values[pos]; // todo: think about deletion
        values[pos] = new V(value);
    }

    void insert_non_full(const K& key, const V& value) {
        int pos = find_key_pos(key);
        if (is_leaf) {
            for (int i = used_keys; i > pos; --i) {
                keys[i] = keys[i - 1];
                values[i] = values[i - 1];
            }
            keys[pos] = key;
            values[pos] = new V(value);
            used_keys += 1;
        } else {
            if (children[pos]->used_keys == max_key_num) {
                split_child(pos, children[pos]);
                if (keys[pos] < key) {
                    pos++;
                }
            }
            children[pos]->insert_non_full(key, value);
        }
    }

    void split_child(const int& pos, Node* node) {
        // Create a new node to store (t-1) keys of divided node
        Node* new_node = new Node(node->is_leaf);
        new_node->used_keys = t - 1;

        // Copy the last (t-1) keys of divided node to new_node
        for (int i = 0; i < t - 1; ++i) {
            new_node->keys[i] = node->keys[i + t];
            new_node->values[i] = node->values[i + t];
        }
        // Copy the last (t-1) children of divided node to new_node
        if (!node->is_leaf) {
            for (int i = 0; i < t; ++i) {
                new_node->children[i] = node->children[i + t];
            }
        }
        // Reduce the number of keys in the divided node
        node->used_keys = t;

        // Shift children, keys and values to right
        for (int i = used_keys + 1; i >= pos + 2; --i) {
            children[i] = children[i - 1];
        }
        children[pos + 1] = new_node;

        for (int i = used_keys; i >= pos + 1; --i) {
            keys[i] = keys[i - 1];
            values[i] = values[i - 1];
        }
        // set the key-divider
        keys[pos] = node->keys[t - 1];
        ++used_keys;
    }

    void traverse() {
        int i;
        auto traverse_child = [&]() {
            if (!is_leaf) {
                cout << endl;
                children[i]->traverse();
                cout << endl;
            }
        };
        for (i = 0; i < used_keys; ++i) {
            traverse_child();
            cout << keys[i] << " ";
            if (is_leaf) {
                cout << ": " << *values[i] << " | ";
            }
        }
        traverse_child();
    }
};

/** Tree */
template <  typename K,
            typename V,
            int t,
            typename Oit = IstreamIterator
            >
class BTree : IKeyValueStorage<K, V, Oit> {
    using Node = BTreeNode<K, V, t>;

    // Корень дерева содержит от (1 до 2t − 1) ключей (0 или от 2 до 2t детей).
    // Все остальные узлы содержат от (t − 1) до (2t − 1) ключей (от t до 2t детей).
    Node* root = nullptr;

public:
    explicit BTree() : root(new Node(true)) {
        root->is_leaf = true;
    }

    ~BTree() {
        delete root;
    }
    
    Node* find_leaf_node(const K& key) {
        Node* curr = root;
        while (!curr->is_leaf) {
            for (int i = 0; i <= curr->used_keys; ++i) {
                if (i == curr->used_keys || key <= curr->keys[i]) {
                    curr = curr->children[i];
                    break;
                }
            }
        }
        return curr;
    }
    
    bool exist(const K &key) {
        Node* leaf_node = find_leaf_node(key);
        return leaf_node->has_key(key);
    }
    
    void set(const K &key, const V& value) {
        Node* leaf_node = find_leaf_node(key);
        int pos = leaf_node->key_pos_in_leaf_node(key);
        if (pos != leaf_node->used_keys) {
            leaf_node->set(pos, value);
        } else {
            insert(key, value);
        }
    }
    
    bool insert(const K& key, const V& value) override {
        if (root->used_keys == Node::max_key_num) {
            auto new_root = new Node(false);
            new_root->children[0] = root;
            new_root->split_child(0, root);

            // find the children have new key
            int pos = new_root->find_key_pos(key);
            new_root->children[pos]->insert_non_full(key, value);
            root = new_root;
        } else {
            root->insert_non_full(key, value);
        }
        return true;
    }

    Oit get(const K& key) override {
        return std::istream_iterator<char>();
    }

    bool remove(const K& key) override {
        return false;
    }

    void traverse() {
        if (root)
            root->traverse();
    }
};


void testBTree() {
    BTree<int, std::string, 3> treeString;
    BTree<int, char*, 10> treeBlob;
    cout << sizeof(std::unique_ptr<BTreeNode<int, int>>) << endl;
    cout << sizeof(BTreeNode<int, int>*) << endl;
    cout << sizeof(std::unique_ptr<int>) << endl;

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

