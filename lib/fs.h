#ifndef IRIS_FS_H
#define IRIS_FS_H


#include <iostream>
#include <memory>
#include <dirent.h>

namespace fs {

class file {

public:
    file() : loc("") { }
    file(const std::string &path) : loc(path) { }
    const std::string &path() const { return loc; }

    file child(const std::string &name) const {
        if (name.empty() || loc.empty()) {
            return *this;
        }

        std::string maybesep = (name.front() != '/' && loc.back() != '/') ? "/" : "";
        return file(loc + maybesep + name);
    }


private:
    std::string loc;
};

class dir_iterator {
public:
    typedef std::input_iterator_tag iterator_category;
    typedef file value_type;
    typedef ptrdiff_t difference_type;
    typedef const value_type *pointer;
    typedef const value_type &reference;

    dir_iterator(const file &fd) : basepath(fd.path()), dirp(nullptr), entry(nullptr) {

        DIR *dfd = opendir(fd.path().c_str());
        if (dfd) {
            dirp = std::shared_ptr<DIR>(dfd, closedir);
            next();
        }
    }

    dir_iterator() : basepath(), dirp(nullptr), entry(nullptr) { }


    dir_iterator& operator++() {
        next();
        return *this;
    }

    dir_iterator operator++(int) {
        dir_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    file operator*() const {
        return file(basepath).child(entry->d_name);
    }

    bool operator==(const dir_iterator &o) const {
        if (entry == nullptr && o.entry == nullptr) {
            return true;
        } else if(entry != nullptr && o.entry != nullptr &&
                  !strcmp(entry->d_name, o.entry->d_name)) {
            return true;
        }

        return false;
    }

    bool operator!=(const dir_iterator &o) const {
        return !(*this == o);
    }

private:

    void next() {

        bool show_hidden = true;
        do {
            readdir_r(dirp.get(), &buffer, &entry);
        } while(entry != nullptr && (!show_hidden && entry->d_name[0] == '.'));
    }

private:
    std::string basepath;
    std::shared_ptr<DIR> dirp;
    struct dirent *entry;
    struct dirent  buffer;
};


}


#endif