#pragma once

#include "btree_node.h"
#include "btree.h"

template<class K, class V>
BTreeNodeStore<K, V> ::BTreeNodeStore(const int& t, bool isLeaf) :
    nCurrentEntry(0),
    t(t),
    flag(isLeaf ? 1 : 0)
{
    int size = max_child_num();
    this->arrayPosKey = new int[size - 1];
    this->arrayPosChild = new int [size];
    for (int i = 0; i < size - 1; ++i) {
        this->arrayPosKey[i] = -1;
        this->arrayPosChild[i] = -1;
    }
    this->arrayPosChild[size - 1] = -1;
}

template<class K, class V>
BTreeNodeStore<K, V> ::~BTreeNodeStore() {
    if (this->arrayPosKey != NULL) {
        delete[] this->arrayPosKey;
        this->arrayPosKey = NULL;
    }

    if (this->arrayPosChild != NULL) {
        delete[] this->arrayPosChild;
        this->arrayPosChild = NULL;
    }

}

template<class K, class V>
bool BTreeNodeStore<K, V>::checkIsLeaf() const {
    return (this->flag & 1) == 1;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::splitChild(BTreeStore<K, V>* bTree, const int &index, BTreeNodeStore<K, V>* &node) {
    BTreeNodeStore<K, V>* newNode = new BTreeNodeStore<K, V>(node->t, node->checkIsLeaf());
    newNode->nCurrentEntry = t - 1;
    for (int i = 0; i < t - 1; i++) {
        newNode->arrayPosKey[i] = node->arrayPosKey[i + t];
        node->arrayPosKey[i + t] = -1;
    }
    if (!node->checkIsLeaf()) {
        for (int i = 0; i < t; i++) {
            newNode->arrayPosChild[i] = node->arrayPosChild[i + t];
            node->arrayPosChild[i + t] = -1;
        }
    }

    bTree->setPosEndFileWrite();
    const int &pos = bTree->getPosFileWrite();
    newNode->m_pos = pos;
    //write new node
    bTree->writeNode(newNode, newNode->m_pos);

    node->nCurrentEntry = t - 1;

    //write node
    bTree->writeNode(node, node->m_pos);

    for (int i = nCurrentEntry; i >= index + 1; --i) {
        arrayPosChild[i + 1] = arrayPosChild[i];
    }
    arrayPosChild[index + 1] = newNode->m_pos;

    for (int i = nCurrentEntry - 1; i >= index; --i) {
        arrayPosKey[i + 1] = arrayPosKey[i];
    }

    arrayPosKey[index] = node->arrayPosKey[t - 1];
    nCurrentEntry = nCurrentEntry + 1;

    //write node
    bTree->writeNode(this, m_pos);
    delete newNode;
}

template<class K, class V>
Entry<K, V>* BTreeNodeStore<K, V> ::getEntry(BTreeStore<K, V>* bTree, const int& i) {
    if (i < 0 || i > nCurrentEntry - 1) {
        return NULL;
    }

    int pos = this->arrayPosKey[i];
    Entry<K, V>* entry = new Entry<K, V>();

    //read entry
    bTree->readEntry(entry, pos);
    return entry;
}

template<class K, class V>
BTreeNodeStore<K, V>* BTreeNodeStore<K, V> ::getBTreeNodeStore(BTreeStore<K, V>* bTree, const int& i) {
    if (i < 0 || i > this->nCurrentEntry) {
        return NULL;
    }

    int pos = this->arrayPosChild[i];
    BTreeNodeStore<K, V>* node = new BTreeNodeStore(this->t, false);

    //read node
    bTree->readNode(node, pos);
    return node;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::insertNotFull(BTreeStore<K, V>* bTree, const Entry<K, V>* entry) {
    int i = this->nCurrentEntry - 1;
    Entry<K, V>* entryTmp = getEntry(bTree, i);
    int pos;

    if (this->checkIsLeaf()) {
        while (i >= 0 && entryTmp != NULL && entryTmp->getKey() > entry->getKey()) {
            arrayPosKey[i + 1] = arrayPosKey[i];
            i--;
            delete entryTmp;
            entryTmp = getEntry(bTree, i);
        }

        bTree->setPosEndFileWrite();
        pos = bTree->getPosFileWrite();

        //write entry
        bTree->writeEntry(entry, pos);

        arrayPosKey[i + 1] = pos;
        nCurrentEntry = nCurrentEntry + 1;

        //write node
        bTree->writeNode(this, m_pos);
    } else {
        //nho dung binary search
        //        while (i >= 0 && entryTmp != NULL && this->myCompare->compareKey(entryTmp->getKey(),
        //                entry->getKey()) > 0) {
        //            i--;
        //            delete entryTmp;
        //            entryTmp = this->getEntry(bTree, i);
        //        }

        i = findKeyBinarySearch(bTree, entry->getKey());

        BTreeNodeStore<K, V>* node = getBTreeNodeStore(bTree, i);

        if (node->nCurrentEntry == 2 * t - 1) {
            splitChild(bTree, i, node);

            entryTmp = getEntry(bTree, i);

            if (entryTmp->getKey() < entry->getKey()) {
                i++;
            }
            delete entryTmp;

            delete node;
        }

        node = getBTreeNodeStore(bTree, i);
        node->insertNotFull(bTree, entry);

        delete node;
    }
}

template<class K, class V>
void BTreeNodeStore<K, V> ::traverse(BTreeStore<K, V>* bTree) {
    int i;
    BTreeNodeStore<K, V>* node;
    Entry<K, V>* entry;
    for (i = 0; i < this->nCurrentEntry; ++i) {
        if (!checkIsLeaf()) {
            node = getBTreeNodeStore(bTree, i);
            cout << endl;
            node->traverse(bTree);
            cout << endl;
            delete node;
        }
        entry = getEntry(bTree, i);
        cout << "[key]: " << entry->key << " - [value]: " << entry->value << " ";
        delete entry;
    }
    if (!this->checkIsLeaf()) {
        node = this->getBTreeNodeStore(bTree, i);
        cout << endl;
        node->traverse(bTree);
        cout << endl;
        delete node;
    }
}

template<class K, class V>
int BTreeNodeStore<K, V> ::findKeyBinarySearch(BTreeStore<K, V>*bTree, const K& key) {
    int low = 0;
    int hight = this->nCurrentEntry - 1;
    int middle = (low + hight) / 2;
    Entry<K, V>* entry = this->getEntry(bTree, middle);
    while (low <= hight) {
        if (entry->getKey() == key) {
            delete entry;
            return middle;
        } else if (entry->getKey() > key) {
            hight = middle - 1;
        } else {
            low = middle + 1;
        }
        delete entry;
        middle = (low + hight) / 2;
        entry = getEntry(bTree, middle);
    }
    delete entry;
    return hight + 1;
}

//su dung ham search nho delete con tro tra ve

template<class K, class V>
Entry<K, V>* BTreeNodeStore<K, V>::search(BTreeStore<K, V>* bTree, const K& key) {
    int i = 0;
    Entry<K, V>* entry = getEntry(bTree, i);
    while (i < nCurrentEntry && entry->getKey() < key) {
        ++i;
        entry = getEntry(bTree, i);
    }

    if (i < nCurrentEntry && entry->getKey() == key) {
        return entry;
    }

    if (checkIsLeaf()) {
        delete entry;
        return NULL;
    }

    return getBTreeNodeStore(bTree, i)->search(bTree, key);
}

template<class K, class V>
bool BTreeNodeStore<K, V> ::set(BTreeStore<K, V>* bTree, const K& key, const V& value) {
    int i = findKeyBinarySearch(bTree, key);
    Entry<K, V>* entry = NULL;

    if (i < this->nCurrentEntry) {
        entry = getEntry(bTree, i);
        if (entry->getKey() == key) {
            bTree->writeFlag('0', arrayPosKey[i]);

            bTree->setPosEndFileWrite();
            int curr_pos = bTree->getPosFileWrite();

            arrayPosKey[i] = curr_pos;
            entry->setValue(value);

            bTree->writeEntry(entry, curr_pos);

            bTree->writeNode(this, m_pos);

            delete entry;
            return true;
        }
    }
    if (this->checkIsLeaf()) {
        delete entry;
        return false;
    }
    if (entry != NULL) {
        delete entry;
        entry = NULL;
    }

    auto node = getBTreeNodeStore(bTree, i);
    bool res = node->set(bTree, key, value);

    delete node;
    return res;
}

template<class K, class V>
bool BTreeNodeStore<K, V>::remove(BTreeStore<K, V>* bTree, const K& key) {
    bool res;

    int index = findKeyBinarySearch(bTree, key);
    Entry<K, V>* entry = getEntry(bTree, index);

    if (index < nCurrentEntry && entry->getKey() == key) {
        res = checkIsLeaf() ? removeFromLeaf(bTree, index) : removeFromNonLeaf(bTree, index);
        bTree->writeNode(this, m_pos);
    } else {
        if (checkIsLeaf()) {
            delete entry;
            return false;
        }

        bool flag = (index == nCurrentEntry);
        auto* child = getBTreeNodeStore(bTree, index); // nho delete node

        if (child->nCurrentEntry < t) {
            fillNode(bTree, index);
        }

        if (flag && index > nCurrentEntry) {
            auto* childPrev = getBTreeNodeStore(bTree, index - 1);
            childPrev->remove(bTree, key);

            childPrev = getBTreeNodeStore(bTree, index - 1);
            if (childPrev != nullptr) {
                bTree->writeNode(childPrev, childPrev->m_pos);
            }
            delete childPrev;
        } else {
            auto *m_child = getBTreeNodeStore(bTree, index);
            m_child->remove(bTree, key);
            delete m_child;
        }

        //write node
        auto * child_2 = getBTreeNodeStore(bTree, index);
        if (child_2 != nullptr) {
            bTree->writeNode(child_2, child_2->m_pos);
        }
        res = true;


        //write node

        delete child;
        delete child_2;
    }

    delete entry;

    return res;
}

template<class K, class V>
bool BTreeNodeStore<K, V> ::removeFromLeaf(BTreeStore<K, V>* bTree, const int& index) {
    bTree->writeFlag('0', this->arrayPosKey[index]); //gan flag danh dau remove

    for (int i = index + 1; i < this->nCurrentEntry; ++i) {
        this->arrayPosKey[i - 1] = this->arrayPosKey[i];
    }
    this->nCurrentEntry--;

    return true;
}

template<class K, class V>
bool BTreeNodeStore<K, V> ::removeFromNonLeaf(BTreeStore<K, V>* bTree, const int& index) {
    Entry<K, V>* entry = getEntry(bTree, index);
    BTreeNodeStore<K, V>* node = getBTreeNodeStore(bTree, index); //nho delete node
    BTreeNodeStore<K, V>* nodeNext = getBTreeNodeStore(bTree, index + 1); // nho delete

    int curr_pos;
    bool res;

    if (node->nCurrentEntry >= t) {
        curr_pos = getPosEntryPred(bTree, index);
        arrayPosKey[index] = curr_pos;

        entry = new Entry<K, V> ();
        bTree->readEntry(entry, curr_pos);

        K key = entry->getKey();
        delete entry;

        res = node->remove(bTree, key);
    } else if (nodeNext->nCurrentEntry >= t) {
        curr_pos = getPosEntrySucc(bTree, index);
        arrayPosKey[index] = curr_pos;

        entry = new Entry<K, V>();
        bTree->readEntry(entry, curr_pos);

        K key = entry->getKey();
        delete entry;

        res = nodeNext->remove(bTree, key);
    } else {
        mergeNode(bTree, index);
        K key = entry->getKey();
        delete entry;

        node = getBTreeNodeStore(bTree, index);
        res = node->remove(bTree, key);
    }

    bTree->writeNode(node, node->m_pos);

    delete node;
    delete nodeNext;

    return res;
}

template<class K, class V>
int BTreeNodeStore<K, V>::getPosEntryPred(BTreeStore<K, V>* bTree, const int& index) {
    //    BTreeNodeStore<K, V>* nodeCurrent = this->getBTreeNodeStore(bTree, index);
    BTreeNodeStore<K, V>* nodeCurrent = new BTreeNodeStore<K, V>(this->t, false);
    bTree->readNode(nodeCurrent, this->arrayPosChild[index]);
    while (!nodeCurrent->checkIsLeaf()) {
        bTree->readNode(nodeCurrent, nodeCurrent->arrayPosChild[nodeCurrent->nCurrentEntry]);
    }

    int pos = nodeCurrent->arrayPosKey[nodeCurrent->nCurrentEntry - 1];
    delete nodeCurrent;

    return pos;
}

template<class K, class V>
int BTreeNodeStore<K, V> ::getPosEntrySucc(BTreeStore<K, V>* bTree, const int& index) {
    //    BTreeNodeStore<K, V>* nodeCurrent = this->getBTreeNodeStore(bTree, index + 1);
    BTreeNodeStore<K, V>* nodeCurrent = new BTreeNodeStore<K, V>(this->t, false);
    bTree->readNode(nodeCurrent, this->arrayPosChild[index + 1]);
    while (!nodeCurrent->checkIsLeaf()) {
        bTree->readNode(nodeCurrent, nodeCurrent->arrayPosChild[0]);
    }

    int pos = nodeCurrent->arrayPosKey[0];
    delete nodeCurrent;

    return pos;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::mergeNode(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V>* child = this->getBTreeNodeStore(bTree, index); //nho delete
    BTreeNodeStore<K, V>* childNext = this->getBTreeNodeStore(bTree, index + 1); //nho delete

    child->arrayPosKey[this->t - 1] = this->arrayPosKey[index];

    for (int i = 0; i < childNext->nCurrentEntry; ++i) {
        child->arrayPosKey[i + this->t] = childNext->arrayPosKey[i];
    }
    if (!child->checkIsLeaf()) {
        for (int i = 0; i <= childNext->nCurrentEntry; ++i) {
            child->arrayPosChild[i + this->t] = childNext->arrayPosChild[i];
        }
    }

    child->nCurrentEntry = child->nCurrentEntry + childNext->nCurrentEntry + 1;

    //write node
    bTree->writeNode(child, this->arrayPosChild[index]);

    childNext->flag = childNext->flag | (1 << 1); //bat co da xoa node nay
    //write node
    bTree->writeNode(childNext, this->arrayPosChild[index + 1]);

    for (int i = index + 1; i < this->nCurrentEntry; ++i) {
        this->arrayPosKey[i - 1] = this->arrayPosKey[i];
    }
    for (int i = index + 1; i < this->nCurrentEntry; ++i) {

        this->arrayPosChild[i] = this->arrayPosChild[i + 1];
    }

    this->nCurrentEntry--;
    bTree->writeNode(this, m_pos);

    delete child;
    delete childNext;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::fillNode(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V>* childPrev = this->getBTreeNodeStore(bTree, index - 1); // nho delete
    BTreeNodeStore<K, V>* childNext = this->getBTreeNodeStore(bTree, index + 1); // nho delete

    if (index != 0 && childPrev->nCurrentEntry >= t) {
        this->borrowFromNodePrev(bTree, index);
    } else if (index != this->nCurrentEntry && childNext->nCurrentEntry >= t) {
        this->borrowFromNodeNext(bTree, index);
    } else {
        if (index != this->nCurrentEntry) {
            this->mergeNode(bTree, index);
        } else {

            this->mergeNode(bTree, index - 1);
        }
    }

    delete childPrev;
    delete childNext;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::borrowFromNodePrev(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V>* childPrev = this->getBTreeNodeStore(bTree, index - 1); // nho delete
    BTreeNodeStore<K, V>* child = this->getBTreeNodeStore(bTree, index); // nho delete
    for (int i = child->nCurrentEntry - 1; i >= 0; --i) {
        child->arrayPosKey[i + 1] = child->arrayPosKey[i];
    }
    if (!child->checkIsLeaf()) {
        for (int i = child->nCurrentEntry; i >= 0; --i) {
            child->arrayPosChild[i + 1] = child->arrayPosChild[i];
        }
    }

    child->arrayPosKey[0] = this->arrayPosKey[index - 1];
    if (!child->checkIsLeaf()) {

        child->arrayPosChild[0] = childPrev->arrayPosChild[childPrev->nCurrentEntry];
    }
    this->arrayPosKey[index - 1] = childPrev->arrayPosKey[childPrev->nCurrentEntry - 1];
    child->nCurrentEntry++;
    childPrev->nCurrentEntry--;

    bTree->writeNode(child, child->m_pos);
    bTree->writeNode(childPrev, childPrev->m_pos);
    bTree->writeNode(this, m_pos);

    delete childPrev;
    delete child;
}

template<class K, class V>
void BTreeNodeStore<K, V> ::borrowFromNodeNext(BTreeStore<K, V>* bTree, const int& index) {
    BTreeNodeStore<K, V>* childNext = this->getBTreeNodeStore(bTree, index + 1); // nho delete
    BTreeNodeStore<K, V>* child = this->getBTreeNodeStore(bTree, index); // nho delete
    child->arrayPosKey[child->nCurrentEntry] = this->arrayPosKey[index];

    if (!child->checkIsLeaf()) {
        child->arrayPosChild[child->nCurrentEntry + 1] = childNext->arrayPosChild[0];
    }

    this->arrayPosKey[index] = childNext->arrayPosKey[0];
    for (int i = 1; i < childNext->nCurrentEntry; ++i) {
        childNext->arrayPosKey[i - 1] = childNext->arrayPosKey[i];
    }
    if (!childNext->checkIsLeaf()) {
        for (int i = 1; i <= childNext->nCurrentEntry; ++i) {
            childNext->arrayPosChild[i - 1] = childNext->arrayPosChild[i];
        }
    }
    child->nCurrentEntry++;
    childNext->nCurrentEntry--;

    bTree->writeNode(child, child->m_pos);
    bTree->writeNode(childNext, childNext->m_pos);
    bTree->writeNode(this, m_pos);

    delete childNext;
    delete child;

}
