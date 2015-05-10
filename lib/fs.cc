
#include <fs.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>

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


fn_matcher::fn_matcher(const std::string pattern, int flags)
        :pattern(pattern), flags(flags) {

}

bool fn_matcher::operator()(const std::string &str_to_match) {
    int res = ::fnmatch(pattern.c_str(), str_to_match.c_str(), flags);

    if (res == 0) {
        return true;
    } else if (res == FNM_NOMATCH) {
        return false;
    } else {
        throw std::runtime_error("Error while calling fnmatch(3)");
    }
}


bool fn_matcher::operator()(const fs::file &the_file) {
    return this->operator()(the_file.path()); //FIXME: should be name()
}
}