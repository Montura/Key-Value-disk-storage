#pragma once

template <typename K, typename V>
struct Entry {
    K* key;
    V* value;

    Entry() {
        this->key = new K();
        this->value = new V();
    }

    Entry(const K &key, const V &value) : key(new K()), value(new V()) {
        *(this->key) = key;
        *(this->value) = value;
    }

    Entry(const Entry<K, V>* entry) : key(new K()), value(new V()) {
        *(this->key) = entry->getKey();
        *(this->value) = entry->getValue();
    }

    void setKeyValue(const K &key, const V &value) {
        *(this->key) = key;
        *(this->value) = value;
    }

    K getKey() const {
        return *(this->key);
    }

    void setKey(const K &key) {
        *(this->key) = key;
    }

    V getValue() const {
        return *(this->value);
    }

    const V* getPointValue() {
        return this->value;
    }

    void setValue(const V &value) {
        *(this->value) = value;
    }
};