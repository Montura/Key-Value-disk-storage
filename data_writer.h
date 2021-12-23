#pragma once

#include <string>
#include <iostream>

class DataOutput {
public:
    DataOutput();
    virtual ~DataOutput();
    virtual void writeByte(uint8_t b) = 0;
    virtual void writeBytes(const uint8_t* b, size_t offset, size_t length) = 0;

public:
    void writeString(const std::string& s);
    void writeInt(int32_t i);
    void writeArrayInt(const int* arr, int len);
    virtual void flush();
};

class DataInput {
public:
    DataInput();
    virtual ~DataInput();
    virtual void readByte(uint8_t& b) = 0;
    virtual void readBytes(uint8_t* b, size_t offset, size_t length) = 0;
    virtual void resetBuffer() = 0;
public:
    void readString(std::string& s);
    void readInt(int32_t& i);
    void readArrayInt(int* arri, int len);
};