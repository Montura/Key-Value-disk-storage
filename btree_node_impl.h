#pragma once

#include "btree_node.h"
#include "btree.h"

template<class K, class V>
BTreeNodeStore<K, V> ::BTreeNodeStore(const int& t, bool isLeaf) :
    nCurrentEntry(0),
    t(t),
    flag(isLeaf ? 1 : 0),
    arrayPosKey(max_key_num(), -1),
    arrayPosChild(max_child_num(), -1)
{}

template<class K, class V>
BTreeNodeStore<K, V> ::~BTreeNodeStore() {}

template<class K, class V>
bool BTreeNodeStore<K, V>::checkIsLeaf() const {
    return (flag & 1) == 1;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::splitChild(BTreeStore<K, V>* bTree, const int &index, BTreeNodeStore<K, V> &node) {
    BTreeNodeStore<K, V> new_node(node.t, node.checkIsLeaf());
    new_node.nCurrentEntry = t - 1;
    for (int i = 0; i < t - 1; i++) {
        new_node.arrayPosKey[i] = node.arrayPosKey[i + t];
        node.arrayPosKey[i + t] = -1;
    }
    if (!node.checkIsLeaf()) {
        for (int i = 0; i < t; i++) {
            new_node.arrayPosChild[i] = node.arrayPosChild[i + t];
            node.arrayPosChild[i + t] = -1;
        }
    }

    bTree->setPosEndFileWrite();
    new_node.m_pos = bTree->getPosFileWrite();
    //write new node
    bTree->writeNode(new_node, new_node.m_pos);

    node.nCurrentEntry = t - 1;

    //write node
    bTree->writeNode(node, node.m_pos);

    for (int i = nCurrentEntry; i >= index + 1; --i) {
        arrayPosChild[i + 1] = arrayPosChild[i];
    }
    arrayPosChild[index + 1] = new_node.m_pos;

    for (int i = nCurrentEntry - 1; i >= index; --i) {
        arrayPosKey[i + 1] = arrayPosKey[i];
    }

    arrayPosKey[index] = node.arrayPosKey[t - 1];
    nCurrentEntry = nCurrentEntry + 1;

    //write node
    bTree->writeNode(*this, m_pos);
}

template<class K, class V>
Entry<K, V> BTreeNodeStore<K, V> ::getEntry(BTreeStore<K, V>* bTree, const int& i) {
    if (i < 0 || i > nCurrentEntry - 1) {
        return {}; // nullptr
    }

    int pos = arrayPosKey[i];
    Entry<K, V> entry;

    //read entry
    bTree->readEntry(entry, pos);
    return entry;
}

template<class K, class V>
BTreeNodeStore<K, V> BTreeNodeStore<K, V> ::getBTreeNodeStore(BTreeStore<K, V>* bTree, const int& i) {
    if (i < 0 || i > nCurrentEntry) {
        return BTreeNodeStore<K,V>(1, false); // dummy
    }

    int pos = arrayPosChild[i];
    BTreeNodeStore<K, V> node(t, false);

    //read node
    bTree->readNode(node, pos);
    return node;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::insertNotFull(BTreeStore<K, V>* bTree, const Entry<K, V>& entry) {
    int i = nCurrentEntry - 1;
    Entry<K, V> entryTmp = getEntry(bTree, i);
    int pos;

    if (checkIsLeaf()) {
        while (i >= 0 && !entryTmp.is_dummy() && entryTmp.key > entry.key) {
            arrayPosKey[i + 1] = arrayPosKey[i];
            i--;
            entryTmp = getEntry(bTree, i);
        }

        bTree->setPosEndFileWrite();
        pos = bTree->getPosFileWrite();

        //write entry
        bTree->writeEntry(entry, pos);

        arrayPosKey[i + 1] = pos;
        nCurrentEntry = nCurrentEntry + 1;

        //write node
        bTree->writeNode(*this, m_pos);
    } else {
        i = findKeyBinarySearch(bTree, entry.key);

        BTreeNodeStore<K, V> node = getBTreeNodeStore(bTree, i);

        if (node.nCurrentEntry == 2 * t - 1) {
            splitChild(bTree, i, node);

            entryTmp = getEntry(bTree, i);

            if (entryTmp.key < entry.key) {
                i++;
            }
        }

        node = getBTreeNodeStore(bTree, i);
        node.insertNotFull(bTree, entry);
    }
}

template<class K, class V>
void BTreeNodeStore<K, V> ::traverse(BTreeStore<K, V>* bTree) {
    int i;
    BTreeNodeStore<K, V> node;
    Entry<K, V> entry;
    for (i = 0; i < nCurrentEntry; ++i) {
        if (!checkIsLeaf()) {
            node = getBTreeNodeStore(bTree, i);
            cout << endl;
            node.traverse(bTree);
            cout << endl;
            delete node;
        }
        entry = getEntry(bTree, i);
        cout << "[key]: " << entry.key << " - [value]: " << entry.value << " ";
        delete entry;
    }
    if (!checkIsLeaf()) {
        node = getBTreeNodeStore(bTree, i);
        cout << endl;
        node.traverse(bTree);
        cout << endl;
        delete node;
    }
}

template<class K, class V>
int BTreeNodeStore<K, V> ::findKeyBinarySearch(BTreeStore<K, V>*bTree, const K& key) {
    int low = 0;
    int hight = nCurrentEntry - 1;
    int middle = (low + hight) / 2;
    Entry<K, V> entry = getEntry(bTree, middle);
    while (low <= hight) {
        if (entry.key == key) {
            return middle;
        } else {
            if (entry.key > key) {
                hight = middle - 1;
            } else {
                low = middle + 1;
            }
        }
        middle = (low + hight) / 2;
        entry = getEntry(bTree, middle);
    }
    return hight + 1;
}

//su dung ham search nho delete con tro tra ve

template<class K, class V>
Entry<K, V> BTreeNodeStore<K, V>::search(BTreeStore<K, V>* bTree, const K& key) {
    int i = 0;
    Entry<K, V> entry = getEntry(bTree, i);
    while (i < nCurrentEntry && entry.key < key) {
        ++i;
        entry = getEntry(bTree, i);
    }

    if (i < nCurrentEntry && entry.key == key) {
        return entry;
    }

    if (checkIsLeaf()) {
        return {}; // nullptr dummy
    }

    return getBTreeNodeStore(bTree, i).search(bTree, key);
}

template<class K, class V>
bool BTreeNodeStore<K, V> ::set(BTreeStore<K, V>* bTree, const K& key, const V& value) {
    int i = findKeyBinarySearch(bTree, key);
    Entry<K, V> entry;

    if (i < nCurrentEntry) {
        entry = getEntry(bTree, i);
        if (entry.key == key) {
            bTree->writeFlag('0', arrayPosKey[i]);

            bTree->setPosEndFileWrite();
            int curr_pos = bTree->getPosFileWrite();

            arrayPosKey[i] = curr_pos;
            entry.value = value;

            bTree->writeEntry(entry, curr_pos);

            bTree->writeNode(*this, m_pos);

            return true;
        }
    }
    if (checkIsLeaf()) {
        return false;
    }

    auto node = getBTreeNodeStore(bTree, i);
    bool res = node.set(bTree, key, value);

    return res;
}

template<class K, class V>
bool BTreeNodeStore<K, V>::remove(BTreeStore<K, V>* bTree, const K& key) {
    bool res;

    int index = findKeyBinarySearch(bTree, key);
    Entry<K, V> entry = getEntry(bTree, index);

    if (index < nCurrentEntry && entry.key == key) {
        res = checkIsLeaf() ? removeFromLeaf(bTree, index) : removeFromNonLeaf(bTree, index);
        bTree->writeNode(*this, m_pos);
    } else {
        if (checkIsLeaf()) {
            return false;
        }

        bool flag = (index == nCurrentEntry);
        auto child = getBTreeNodeStore(bTree, index); // nho delete node

        if (child.nCurrentEntry < t) {
            fillNode(bTree, index);
        }

        if (flag && index > nCurrentEntry) {
            auto childPrev = getBTreeNodeStore(bTree, index - 1);
            childPrev.remove(bTree, key);

            childPrev = getBTreeNodeStore(bTree, index - 1);
            if (childPrev.t != 1) {
                bTree->writeNode(childPrev, childPrev.m_pos);
            }
        } else {
            auto m_child = getBTreeNodeStore(bTree, index);
            m_child.remove(bTree, key);
        }

        //write node
        auto child_2 = getBTreeNodeStore(bTree, index);
        if (child_2.t != 1) {
            bTree->writeNode(child_2, child_2.m_pos);
        }
        res = true;
    }

    return res;
}

template<class K, class V>
bool BTreeNodeStore<K, V> ::removeFromLeaf(BTreeStore<K, V>* bTree, const int& index) {
    bTree->writeFlag('0', arrayPosKey[index]); //gan flag danh dau remove

    for (int i = index + 1; i < nCurrentEntry; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    nCurrentEntry--;

    return true;
}

template<class K, class V>
bool BTreeNodeStore<K, V> ::removeFromNonLeaf(BTreeStore<K, V>* bTree, const int& index) {
    Entry<K, V> entry = getEntry(bTree, index);
    BTreeNodeStore<K, V> node = getBTreeNodeStore(bTree, index); //nho delete node
    BTreeNodeStore<K, V> nodeNext = getBTreeNodeStore(bTree, index + 1); // nho delete

    int curr_pos;
    bool res;

    if (node.nCurrentEntry >= t) {
        curr_pos = getPosEntryPred(bTree, index);
        arrayPosKey[index] = curr_pos;

        entry = Entry<K, V> ();
        bTree->readEntry(entry, curr_pos);

        K key = entry.key;

        res = node.remove(bTree, key);
    } else if (nodeNext.nCurrentEntry >= t) {
        curr_pos = getPosEntrySucc(bTree, index);
        arrayPosKey[index] = curr_pos;

        entry = Entry<K, V>();
        bTree->readEntry(entry, curr_pos);

        K key = entry.key;

        res = nodeNext.remove(bTree, key);
    } else {
        mergeNode(bTree, index);
        K key = entry.key;

        node = getBTreeNodeStore(bTree, index);
        res = node.remove(bTree, key);
    }

    bTree->writeNode(node, node.m_pos);

    return res;
}

template<class K, class V>
int BTreeNodeStore<K, V>::getPosEntryPred(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V> nodeCurrent(t, false);
    bTree->readNode(nodeCurrent, arrayPosChild[index]);
    while (!nodeCurrent.checkIsLeaf()) {
        bTree->readNode(nodeCurrent, nodeCurrent.arrayPosChild[nodeCurrent.nCurrentEntry]);
    }

    int pos = nodeCurrent.arrayPosKey[nodeCurrent.nCurrentEntry - 1];

    return pos;
}

template<class K, class V>
int BTreeNodeStore<K, V> ::getPosEntrySucc(BTreeStore<K, V>* bTree, const int& index) {
    //    BTreeNodeStore<K, V>* nodeCurrent = getBTreeNodeStore(bTree, index + 1);
    BTreeNodeStore<K, V> nodeCurrent(t, false);
    bTree->readNode(nodeCurrent, arrayPosChild[index + 1]);
    while (!nodeCurrent.checkIsLeaf()) {
        bTree->readNode(nodeCurrent, nodeCurrent.arrayPosChild[0]);
    }

    int pos = nodeCurrent.arrayPosKey[0];

    return pos;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::mergeNode(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V> child = getBTreeNodeStore(bTree, index); //nho delete
    BTreeNodeStore<K, V> childNext = getBTreeNodeStore(bTree, index + 1); //nho delete

    child.arrayPosKey[t - 1] = arrayPosKey[index];

    for (int i = 0; i < childNext.nCurrentEntry; ++i) {
        child.arrayPosKey[i + t] = childNext.arrayPosKey[i];
    }
    if (!child.checkIsLeaf()) {
        for (int i = 0; i <= childNext.nCurrentEntry; ++i) {
            child.arrayPosChild[i + t] = childNext.arrayPosChild[i];
        }
    }

    child.nCurrentEntry += childNext.nCurrentEntry + 1;

    //write node
    bTree->writeNode(child, arrayPosChild[index]);

    childNext.flag = childNext.flag | (1 << 1); //bat co da xoa node nay
    //write node
    bTree->writeNode(childNext, arrayPosChild[index + 1]);

    for (int i = index + 1; i < nCurrentEntry; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    for (int i = index + 1; i < nCurrentEntry; ++i) {

        arrayPosChild[i] = arrayPosChild[i + 1];
    }

    nCurrentEntry--;
    bTree->writeNode(*this, m_pos);

}

template<class K, class V>
void BTreeNodeStore<K, V> ::fillNode(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V> childPrev = getBTreeNodeStore(bTree, index - 1); // nho delete
    BTreeNodeStore<K, V> childNext = getBTreeNodeStore(bTree, index + 1); // nho delete

    if (index != 0 && childPrev.nCurrentEntry >= t) {
        borrowFromNodePrev(bTree, index);
    } else if (index != nCurrentEntry && childNext.nCurrentEntry >= t) {
        borrowFromNodeNext(bTree, index);
    } else {
        if (index != nCurrentEntry) {
            mergeNode(bTree, index);
        } else {

            mergeNode(bTree, index - 1);
        }
    }
}

template<class K, class V>
void BTreeNodeStore<K, V> ::borrowFromNodePrev(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V> childPrev = getBTreeNodeStore(bTree, index - 1); // nho delete
    BTreeNodeStore<K, V> child = getBTreeNodeStore(bTree, index); // nho delete
    for (int i = child.nCurrentEntry - 1; i >= 0; --i) {
        child.arrayPosKey[i + 1] = child.arrayPosKey[i];
    }
    if (!child.checkIsLeaf()) {
        for (int i = child.nCurrentEntry; i >= 0; --i) {
            child.arrayPosChild[i + 1] = child.arrayPosChild[i];
        }
    }

    child.arrayPosKey[0] = arrayPosKey[index - 1];
    if (!child.checkIsLeaf()) {

        child.arrayPosChild[0] = childPrev.arrayPosChild[childPrev.nCurrentEntry];
    }
    arrayPosKey[index - 1] = childPrev.arrayPosKey[childPrev.nCurrentEntry - 1];
    child.nCurrentEntry++;
    childPrev.nCurrentEntry--;

    bTree->writeNode(child, child.m_pos);
    bTree->writeNode(childPrev, childPrev.m_pos);
    bTree->writeNode(*this, m_pos);
}

template<class K, class V>
void BTreeNodeStore<K, V> ::borrowFromNodeNext(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V> childNext = getBTreeNodeStore(bTree, index + 1); // nho delete
    BTreeNodeStore<K, V> child = getBTreeNodeStore(bTree, index); // nho delete

    child.arrayPosKey[child.nCurrentEntry] = arrayPosKey[index];

    if (!child.checkIsLeaf()) {
        child.arrayPosChild[child.nCurrentEntry + 1] = childNext.arrayPosChild[0];
    }

    arrayPosKey[index] = childNext.arrayPosKey[0];
    for (int i = 1; i < childNext.nCurrentEntry; ++i) {
        childNext.arrayPosKey[i - 1] = childNext.arrayPosKey[i];
    }
    if (!childNext.checkIsLeaf()) {
        for (int i = 1; i <= childNext.nCurrentEntry; ++i) {
            childNext.arrayPosChild[i - 1] = childNext.arrayPosChild[i];
        }
    }
    child.nCurrentEntry++;
    childNext.nCurrentEntry--;

    bTree->writeNode(child, child.m_pos);
    bTree->writeNode(childNext, childNext.m_pos);
    bTree->writeNode(*this, m_pos);
}
