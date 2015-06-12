
#include <fs.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#include <libgen.h>
#include <pwd.h>
#include <vector>
#include <fstream>

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
    if (loc.empty() || path_is_absolute(loc)) {
        return;
    }

    // path is not absolute, and not empty

    if (loc.size() > 1 && loc[0] == '~' && loc[1] == '/') {
        file home = home_directory();
        const char *p = loc.c_str() + 2;
        file child = home.child(p);
        loc = child.path();
    } else {
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


std::pair<std::string, std::string> file::splitext() const {
    std::string nm = name();
    std::string::size_type pos = nm.rfind(".");

    std::string root = nm.substr(0, pos);
    std::string ext;

    if (pos != std::string::npos) {
        ext = nm.substr(pos + 1);
    }

    return std::make_pair(root, ext);
}

file file::parent() const {

    if (loc.size() == 1 && loc[0] == '/') {
        return fs::file();
    }

    char *p = strdup(loc.c_str());
    char *n = dirname(p);

    if (n == nullptr || n[0] == '.') {
        return fs::file();
    }

    std::string fn(n);
    free(p);

    return fs::file(fn);
}

file file::make_dir(const std::string &path) {

    int res = ::mkdir(path.c_str(), 0777);

    if (res) {
        throw std::runtime_error("Could not create directory");
    }

    return file(path);
}

bool file::access(int mode) {
    return ::access(loc.c_str(), mode) == 0;
}

bool file::exists() const {
    struct stat buf;
    int res = stat(loc.c_str(), &buf);
    return res == 0;
}


bool file::is_directory() const {
    struct stat buf;
    int res = stat(loc.c_str(), &buf);
    return res == 0 && S_ISDIR(buf.st_mode);
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

    std::string p = std::string(buffer.data());

    if (!p.empty() && p[0] != '/') {
        return parent().child(p);
    }

    return file(p);
}


std::fstream file::stream(std::ios_base::openmode mode) const {
    return std::fstream(loc, mode);
}

std::string file::read_all() const {
    std::ifstream fd(loc, std::ios::in | std::ios::ate);

    if (!fd.is_open()) {
        throw std::runtime_error("Could not open file for reading");
    }

    std::streampos size = fd.tellg();
    std::vector<char> data(static_cast<size_t>(size));
    fd.seekg(0, std::ios::beg);
    fd.read(data.data(), data.size());

    if (!fd.good()) {
        throw std::runtime_error("Error while reading data from file");
    }

    return std::string(data.data(), data.size());
}


void file::write_all(const std::string &data) {

    bool file_exists = exists();
    // todo: do atomic replace
    //       make temp with mkstemp, write, rename
    std::string filepath = loc;

    if (file_exists) {
        char buffer[1024] = {0, };
        fs::file parent_dir = parent();
        snprintf(buffer, sizeof(buffer), "%s/.%sXXXXXX", parent_dir.path().c_str(), name().c_str());
        filepath = mktemp(buffer);
    }

    std::ofstream fd(filepath, std::ios::binary | std::ios::trunc);
    fd.write(data.c_str(), data.size());
    fd.close();

    if (!fd.good()) {
        // hmm, clear out temporary file
        throw std::runtime_error("Error wile writing data to file");
    }

    if (file_exists) {
        int res = rename(filepath.c_str(), path().c_str());
        if (res != 0) {
            unlink(filepath.c_str()); //ignore errors, can't do much
            throw std::runtime_error("Atomic IO failed (rename)");
        }
    }

}


void file::copy(fs::file &dest, bool overwrite) const {

    if (dest.exists() && !overwrite) {
        throw std::runtime_error("destination exists");
    }

    //FIXME: ok, this is really, really lame

    std::string data = read_all();
    dest.write_all(data);
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


bool file::mkdir() {
    int res = ::mkdir(loc.c_str(), S_IRWXU);
    bool have_error = res != 0;

    if (have_error && errno != EEXIST) {
        throw std::runtime_error("Could not create directory");
    }

    return !have_error;
}

bool file::mkdir_with_parents() {
    std::vector<fs::file> parents;
    fs::file iter = parent();
    while (! iter.loc.empty()) {
        parents.push_back(iter);
        iter = iter.parent();
    }

    for (fs::file cf : parents) {
        if (!cf.exists()) {
            cf.mkdir();
        }
    }

    return mkdir();
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