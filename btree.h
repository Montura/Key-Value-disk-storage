#include <vector>
#include <cstdint>
#include <memory>
#include <string>

/** Common Interface */
template <typename Key, typename Value, typename Oit>
struct IKeyValueStorage {
    virtual void insert(const Key& key, const Value& value) = 0;
    virtual Oit get(const Key& key) = 0;
    virtual bool remove(const Key& key) = 0;

    virtual ~IKeyValueStorage() { };
};


/** Node */
template <typename K, typename V, typename Comparator = std::less<K> >
class BTreeNode {
    bool isLeaf = 0;
//    std::vector<K> keys;                // in file
    std::vector<std::pair<K, V>> entries; // in memory
    std::vector<BTreeNode<K, V, Comparator>> children;
    uint8_t order;
    int nCurrentEntry;

public:
    explicit BTreeNode(const int& t, bool isLeaf):
        isLeaf(isLeaf),
        order(t),
        entries(2 * t - 1, -1),
        children(2 * t, -1)
    {}

    explicit BTreeNode(const int& t):
        order(t),
        entries(2 * t - 1, -1),
        children(2 * t, -1)
    {}

    ~BTreeNode() {}

private:
    auto search(const K& key) const {
        auto it = std::find(entries.cbegin(), entries.cend(), key);
        if (it != end(entries)) {
            return it;
        }
        return isLeaf ? end(entries) : children[*it]->search(key);
    }
};


/** Tree */
typedef std::istream_iterator<char> IstreamIterator;

template <  typename K,
            typename V,
            typename Oit = IstreamIterator,
            typename Comparator = std::less<K>,
            int default_order = 25
            >
class BTree : IKeyValueStorage<K, V, Oit> {
    // Корень дерева содержит от (1 до 2t − 1) ключей (0 или от 2 до 2t детей).
    // Все остальные узлы содержат от (order − 1) до (2t − 1) ключей (от order до 2t детей).
    BTreeNode<K, V, Comparator>* root = nullptr;
    uint8_t order; // order

public:
    explicit BTree() {}

    ~BTree() {}

    void insert(const K& key, const V& value) override {}

    Oit get(const K& key) override {
        return std::istream_iterator<char>();
    }

    bool remove(const K& key) override {
        return false;
    }
};

void testBTree() {
    BTree<int, int> tree;
    BTree<int, std::string> treeString;
    BTree<int, char*> treeBlob;

    auto bTree = std::make_unique<BTree<int, int>>();
    for (int i = 0; i < 10; i++) {
        bTree->insert(i, i);
    }

}