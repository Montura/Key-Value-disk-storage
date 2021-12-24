#include "byte_data.h"

#include <unistd.h>
#include <iostream>

BytesDataInputStore::BytesDataInputStore(int n, const int &fd):
    buffer(new char[n]),
    lenBuffer(0),
    offsetBuffer(0),
    fd(fd),
    max(n)
{}

BytesDataInputStore::~BytesDataInputStore() {
    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }
}

void BytesDataInputStore::readByte(uint8_t& b) {
    readBytes(&b, 0, 1);
}

void BytesDataInputStore::readBytes(uint8_t* b, std::size_t offset, std::size_t length) {
    int remain = static_cast<int>(length);
    int minByteRead;
    int remainBuffer;
    while (remain > 0) {
        remainBuffer = lenBuffer - offsetBuffer;
        minByteRead = remain > remainBuffer ? remainBuffer : remain;
        memcpy(b + offset, buffer + offsetBuffer, minByteRead);
        remain -= minByteRead;
        offset += minByteRead;
        offsetBuffer += minByteRead;
        if (offsetBuffer == lenBuffer) {
            readBuffer();
        }
    }
}

void BytesDataInputStore::readBuffer() {
    lenBuffer = read(fd, buffer, max); // unistd.h
    offsetBuffer = 0;
}

void BytesDataInputStore::resetBuffer() {
    lenBuffer = 0;
    offsetBuffer = 0;
}

BytesDataOutputStore::BytesDataOutputStore(int n, const int &fd):
    buffer(new char[n]),
    lenBuffer(n),
    offsetBuffer(0),
    fd(fd)
{}

BytesDataOutputStore::~BytesDataOutputStore() {
    this->flush();

    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }
}

void BytesDataOutputStore::writeByte(uint8_t b) {
    writeBytes(&b, 0, 1);
}

void BytesDataOutputStore::writeBytes(const uint8_t* b, size_t offset, size_t length) {
    int remain = length;
    int minByteWrite;
    int remainBuffer;
    while (remain > 0) {
        remainBuffer = lenBuffer - offsetBuffer;
        minByteWrite = remain > remainBuffer ? remainBuffer : remain;

        memcpy(buffer + offsetBuffer, b + offset, minByteWrite);

        remain -= minByteWrite;
        offsetBuffer += minByteWrite;
        offset += minByteWrite;

        if (offsetBuffer == lenBuffer) {
            flush();
        }
    }
}

void BytesDataOutputStore::flush() {
    if (offsetBuffer != 0) {
        int b = write(fd, buffer, offsetBuffer);
    }
    offsetBuffer = 0;
}