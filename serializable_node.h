#pragma once

#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>

struct Serializable {
    virtual void serialize(std::ostream &out, const int file_version) = 0;
    virtual void deserialize(std::istream &in, const int file_version) = 0;
    virtual ~Serializable() {}
};


struct SerializableNode : Serializable {
    unsigned idx = 0;
    unsigned prev = 0, next = 0;
    int used_keys = 0;
    std::vector<int> keys;
    std::vector<int> data;

    SerializableNode() = default;

    explicit SerializableNode(int n) : prev(1), next(2), used_keys(n), keys(n, 3), data(n, 4) {}

    void serialize(std::ostream &out, const int file_version = 0) override {
        boost::archive::text_oarchive oa(out);
        // write class instance to archive
        oa << *this;
    };

    void deserialize(std::istream &in, const int file_version = 0) override {
        boost::archive::text_iarchive ia(in);
        ia >> *this;
    };

    bool operator==(const SerializableNode &other) const {
        return (idx == other.idx) &&
               (prev == other.prev) &&
               (next == other.next) &&
               (used_keys == other.used_keys) &&
               (keys == other.keys) &&
               (data == other.data);
    }
};

namespace boost::serialization {
    template<class Archive>
    inline void serialize(Archive &ar, SerializableNode &n, const unsigned int file_version) {
        split_free(ar, n, file_version);
    }

    template<class Archive>
    void save(Archive & ar, const SerializableNode & n, unsigned int version) {
        ar & n.idx;
        ar & n.prev;
        ar & n.next;
        ar & n.used_keys;
        for (int i = 0; i < n.used_keys; ++i) {
            ar & n.keys[i];
            ar & n.data[i];
        }
    }
    template<class Archive>
    void load(Archive & ar, SerializableNode & n, unsigned int version) {
        ar & n.idx;
        ar & n.prev;
        ar & n.next;
        ar & n.used_keys;
        n.keys.resize(n.used_keys);
        n.data.resize(n.used_keys);
        for (int i = 0; i < n.used_keys; ++i) {
            ar & n.keys[i];
            ar & n.data[i];
        }
    }
} // namespace boost