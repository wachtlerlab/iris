#ifndef IRIS_FS_H
#define IRIS_FS_H


#include <iostream>
#include <memory>
#include <dirent.h>
#include <cstddef>
#include <cstring>

namespace fs {

class file;

class dir_iterator {
public:
    typedef std::input_iterator_tag iterator_category;
    typedef file value_type;
    typedef ptrdiff_t difference_type;
    typedef const value_type *pointer;
    typedef const value_type &reference;

    dir_iterator(const file &fd);
    dir_iterator(const std::string &path);
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

    file operator*() const;

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
    void next();

private:
    std::string basepath;
    std::shared_ptr<DIR> dirp;
    struct dirent *entry;
    struct dirent  buffer;
};

class file {

public:
    file() : loc("") { }
    file(const std::string &path);
    const std::string &path() const { return loc; }
    std::string name() const;

    file child(const std::string &name) const {
        if (name.empty() || loc.empty()) {
            return *this;
        }

        std::string maybesep = (name.front() != '/' && loc.back() != '/') ? "/" : "";
        return file(loc + maybesep + name);
    }

    file parent() const;

    bool access(int mode);

    bool exists();

    bool is_directory() const;

    file readlink() const;

    // IO

    std::string read_all() const;
    void write_all(const std::string &data);

    // obtain files

    static file current_directory();
    static file home_directory();
    //


    // *******************
    // directory functions

    bool mkdir();
    struct dir_enum {

        dir_enum(const std::string &path) : path(path) { }
        dir_iterator begin() {  return dir_iterator(path);  }
        dir_iterator end() { return dir_iterator{}; }

        std::string path;
    };

    dir_enum children() const {
        return dir_enum(loc);
    }

    static file make_dir(const std::string &path);

    // *******************
    // path related utils

    static bool path_is_absolute(const std::string &path);

private:
    std::string loc;
};


class fn_matcher {
public:
    fn_matcher(const std::string pattern, int flags = 0);
    bool operator()(const std::string &str_to_match);
    bool operator()(const fs::file &the_file);

private:
    std::string pattern;
    int flags;
};

}


#endif