
#include <fs.h>

#include <sys/stat.h>
#include <unistd.h>

namespace fs {

dir_iterator::dir_iterator(const file &fd) : dir_iterator(fd.path()) {
}

dir_iterator::dir_iterator(const std::string &path) :basepath(path), dirp(nullptr), entry(nullptr) {
    DIR *dfd = opendir(basepath.c_str());
    if (dfd) {
        dirp = std::shared_ptr<DIR>(dfd, closedir);
        next();
    }
}

file dir_iterator::operator*() const {
    return file(basepath).child(entry->d_name);
}

void dir_iterator::next() {

    bool show_hidden = true;

    do {
        readdir_r(dirp.get(), &buffer, &entry);
    } while(entry != nullptr && (!show_hidden && entry->d_name[0] == '.'));
}


/// file

file file::make_dir(const std::string &path) {

    int res = mkdir(path.c_str(), 0777);

    if (res) {
        throw std::runtime_error("Could not create directory");
    }

    return file(path);
}

bool file::access(int mode) {
    return ::access(loc.c_str(), mode) == 0;
}

bool file::exists() {
    struct stat buf;
    int res = stat(loc.c_str(), &buf);
    return res == 0;
}

}