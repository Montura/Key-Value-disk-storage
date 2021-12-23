#pragma once

#include <string>

struct ManageFile {
    int fd;

    virtual int openFile(const std::string &path) = 0;
    virtual int closeFile() const = 0;
    virtual bool isEmptyFile() = 0;

    virtual int getPosFile() const = 0;
    virtual void setPosFile(const int& pos) = 0;
    virtual void setPosEndFile() = 0;
};


struct ManageFileRead : public ManageFile {
    ManageFileRead();
    ~ManageFileRead();
    int openFile(const std::string& path) override;
    int closeFile() const override;

    bool isEmptyFile() override;
    int getPosFile() const override;
    void setPosEndFile() override;
    void setPosFile(const int& pos) override;
};

struct ManageFileWrite : public ManageFile {
    ManageFileWrite();
    ~ManageFileWrite();
    int openFile(const std::string& path) override;
    int closeFile() const override;

    bool isEmptyFile() override;
    int getPosFile() const override;
    void setPosEndFile() override;
    void setPosFile(const int& pos) override;
};
