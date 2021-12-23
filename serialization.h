#pragma once

#include <string>

template<class K, class V>
class Serialization {
public:
    virtual std::string serializationKey(K key) = 0;
    virtual std::string serializationValue(V value) = 0;
};

class StringSerialization : public Serialization<std::string, std::string> {
public:

    std::string serializationKey(std::string key) override {
        int len = key.length();
        //        char* res = new char[len + 4];
        std::string res;
        res.resize(len + 4);

        memcpy((char*) res.c_str(), &len, 4);
        memcpy((char*) res.c_str() + 4, key.c_str(), len);
        return res;
    }

    std::string serializationValue(std::string value) override {
        int len = value.length();
        //        char* res = new char[len + 4];
        std::string res;
        res.resize(len + 4);

        memcpy((char*) res.c_str(), &len, 4);
        memcpy((char*) res.c_str() + 4, value.c_str(), len);
        return res;
    }
};


