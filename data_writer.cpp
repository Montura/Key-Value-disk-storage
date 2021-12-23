
#include "data_writer.h"

using std::string;

DataInput::DataInput() {}

DataInput::~DataInput() {}

void DataInput::readInt(int32_t& i) {
    readBytes(reinterpret_cast<uint8_t*> (&i), 0, sizeof (i));
}

void DataInput::readString(std::string& s) {
    int size = 0;

    readInt(size);
    s.resize(size);

    char* data = const_cast<char*> (s.data());

    readBytes(reinterpret_cast<uint8_t*> (data), 0, size);
}

void DataInput::readArrayInt(int* arri, int len) {
    readBytes(reinterpret_cast<uint8_t*> (arri), 0, len * sizeof (int));
}

DataOutput::DataOutput() {}

DataOutput::~DataOutput() {}

void DataOutput::writeInt(int32_t i) {
//    std::cout << __PRETTY_FUNCTION__ << std::endl;
    writeBytes(reinterpret_cast<const uint8_t*> (&i), 0, sizeof (i));
}

void DataOutput::writeString(const std::string& s) {
    size_t size = s.length();
    writeInt(static_cast<uint32_t> (size));
    writeBytes(reinterpret_cast<const uint8_t*> (s.data()), 0, size);
}

void DataOutput::writeArrayInt(const int* arri, int len) {
//    std::cout << __PRETTY_FUNCTION__ << std::endl;
    writeBytes(reinterpret_cast<const uint8_t*> (arri), 0, len * sizeof (int));
}

void DataOutput::flush() {}