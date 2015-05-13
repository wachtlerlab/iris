
#include <fs.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#include <libgen.h>
#include <pwd.h>
#include <vector>

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

file::file(const std::string &path) : loc(path) {
    if (!path_is_absolute(loc)) {
        file cwd = current_directory();
        file child = cwd.child(loc);
        loc = child.path();
    }

    //TODO: make path canonical

}

std::string file::name() const {
    char *p = strdup(loc.c_str());
    char *n = basename(p);
    std::string res(n);
    free(p);
    return res;
}

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

file file::readlink() const {

    std::vector<char> buffer(1024, 0);

    ssize_t res = -1;
    bool try_again = false;
    do {
        res = ::readlink(loc.c_str(), buffer.data(), buffer.size());

        if (res > 0 && buffer.size() == res) {
            buffer.resize(buffer.size() * 2);
            std::fill(buffer.begin(), buffer.end(), 0);
            try_again = true;
        }

    } while (try_again);

    if (res < 0) {
        throw std::runtime_error("Could not read the link");
    }

    return file(std::string(buffer.data()));
}

file file::current_directory() {
    char buf[PATH_MAX] = {0, };

    char *res = ::getcwd(buf, sizeof(buf));

    if (res == nullptr) {
        throw std::runtime_error("Could not get current work dir");
    }

    //result *must* be an absolute path

    std::string path(res);

    if (!path_is_absolute(path)) {
        throw std::runtime_error("current work dir path is relative!");
    }

    return fs::file(path);
}


file file::home_directory() {
    char *home = getenv("HOME");

    if (home != nullptr) {
        return file(std::string(home));
    }

    long bytes = sysconf(_SC_GETPW_R_SIZE_MAX);

    if (bytes == -1) {
        throw std::runtime_error("Unable do deterime buffer size");
    }

    std::vector<char> buffer(static_cast<size_t>(bytes), 0);

    struct passwd pw, *pw_res;
    int res = getpwuid_r(getuid(), &pw, buffer.data(), buffer.size(), &pw_res);

    if (res != 0) {
        throw std::runtime_error("Unable do obtain getpwuid_r data");
    }

    return fs::file(std::string(pw_res->pw_dir));
}


bool file::path_is_absolute(const std::string &path) {
    return !path.empty() && path[0] == '/';
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
    return this->operator()(the_file.name());
}
}