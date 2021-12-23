/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <sys/stat.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>


using std::cout;
using std::endl;
using std::string;

#include "manage_file.h"

ManageFileRead::ManageFileRead() {
    this->fd = -1;
}

ManageFileRead::~ManageFileRead() {
    int flag = close(this->fd);
    if (flag < 0) {
        cout << "close file error" << endl;
    }
}

int ManageFileRead::openFile(const string& path) {
    int fdr;
    //    fdw = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    fdr = open(path.c_str(), O_RDONLY);
    if (fdr == -1) {
        cout << "Open file error" << endl;
        return -1;
    }

    if (fdr > 0) {
        this->fd = fdr;
    }
    return fdr;
}

int ManageFileRead::closeFile() const {
    int flag = close(this->fd);
    if (flag < 0) {
        cout << "close file error" << endl;
    }
    flag = close(this->fd);
    return flag;
}

bool ManageFileRead::isEmptyFile() {
    struct stat sb;
    fstat(this->fd, &sb);
    return (sb.st_size == 0) ? true : false;
}

int ManageFileRead::getPosFile() const {
    return lseek(this->fd, 0, SEEK_CUR);
}

void ManageFileRead::setPosFile(const int& pos) {
    lseek(this->fd, pos, SEEK_SET);
}

void ManageFileRead::setPosEndFile() {
    lseek(this->fd, 0, SEEK_END);
}


ManageFileWrite::ManageFileWrite() {
    this->fd = -1;
}

ManageFileWrite::~ManageFileWrite() {
    int flag = close(this->fd);
    if (flag < 0) {
        cout << "close file error" << endl;
    }
}

int ManageFileWrite::openFile(const string& path) {
    int fdw;
    fdw = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fdw == -1) {
        cout << "Open file error" << endl;
        return -1;
    }

    if (fdw > 0) {
        this->fd = fdw;
    }
    return fdw;
}

int ManageFileWrite::closeFile() const {
    int flag = close(this->fd);
    if (flag < 0) {
        cout << "close file error" << endl;
    }
    flag = close(this->fd);
    return flag;
}

bool ManageFileWrite::isEmptyFile() {
    struct stat sb;
    fstat(this->fd, &sb);
    return (sb.st_size == 0);
}

int ManageFileWrite::getPosFile() const {
    return lseek(this->fd, 0, SEEK_CUR);
}

void ManageFileWrite::setPosFile(const int& pos) {
    lseek(this->fd, pos, SEEK_SET);
}

void ManageFileWrite::setPosEndFile() {
    lseek(this->fd, 0, SEEK_END);
}