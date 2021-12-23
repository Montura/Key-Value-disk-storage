#pragma once

#include <cstdint>
#include "data_writer.h"

class BytesDataInputStore : public DataInput {
private:
    char* buffer;
    int lenBuffer;
    int offsetBuffer;
    int fd;
    int max;
    void readBuffer();
public:
    BytesDataInputStore(int n, const int &fd);
    ~BytesDataInputStore();
    void readByte(uint8_t& b) override;
    void readBytes(uint8_t* b, size_t offset, size_t length) override;
    void resetBuffer() override;
};

class BytesDataOutputStore : public DataOutput {
private:
    char* buffer;
    int lenBuffer;
    int offsetBuffer;
    int fd;
public:
    BytesDataOutputStore(const int n, const int &fd);
    ~BytesDataOutputStore();
    void writeByte(uint8_t b) override;
    void writeBytes(const uint8_t* b, size_t offset, size_t length) override;
    void flush();
};