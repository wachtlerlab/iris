
#include <fs.h>

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

}