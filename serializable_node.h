#pragma once

#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

struct Serializable {
    virtual void serialize(std::ostream &out) const = 0;
    virtual void deserialize(std::istream &in) = 0;
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

    void serialize(std::ostream &out) const override {
        boost::archive::text_oarchive oa(out);
        // write class instance to archive
        oa << *this;
    };

    void deserialize(std::istream &in) override {
        boost::archive::text_iarchive ia(in);
        ia >> *this;
    };

    bool operator==(const SerializableNode& other) const {
        return  (idx == other.idx) &&
                (prev == other.prev) &&
                (next == other.next) &&
                (used_keys == other.used_keys) &&
                (keys == other.keys) &&
                (data == other.data);
    }
};

namespace boost::serialization {
    template<class Archive>
    void serialize(Archive &ar, SerializableNode &l, const unsigned int version) {
        ar & l.idx;
        ar & l.prev;
        ar & l.next;
        ar & l.used_keys;
        l.keys.resize(l.used_keys);
        l.data.resize(l.used_keys);
        for (int i = 0; i < l.used_keys; ++i) {
            ar & l.keys[i];
            ar & l.data[i];
        }
//        ar & l.keys;
//        ar & l.data;
    }
} // namespace boost