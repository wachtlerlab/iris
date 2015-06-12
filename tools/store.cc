

#include <boost/program_options.hpp>
#include <data.h>

#include <getopt.h>

#include <iostream>
#include <yaml-cpp/yaml.h>

static int cmd_info(int argc, char **argv) {
    iris::data::store store = iris::data::store::default_store();

    std::cout << "location: " << store.location().path() << std::endl;
    std::cout << "version: " << store.version_string() << std::endl;

    return 0;
}

static int import_rgb2lms(iris::data::store &store,
                          fs::file &fd,
                          const std::string &data) {
    iris::data::rgb2lms rgb2lms = store.yaml2rgb2lms(data);

    fs::file f_imported = store.store_rgb2lms(rgb2lms);
    fs::file base = f_imported.parent();
    std::cerr << "[I] stored rgb2lms data [" << rgb2lms.identifier() << "]" << std::endl;

    std::string root, ext;

    std::tie(root, ext) = fd.splitext();

    fs::file cac = fd.parent().child(root + ".cac");
    fs::file msd = fd.parent().child(rgb2lms.dataset);

    if (cac.exists()) {
        std::cerr << "[I] found cac data: " << cac.path() << std::endl;
        fs::file dest = base.child(cac.name());
        if (dest.exists()) {
            std::cerr << "[W] cac already exists. skipping!" << std::endl;
        } else {
            cac.copy(dest);
            std::cerr << "[I] cac file imported!" << std::endl;
        }
    }

    if (msd.exists()) {
        std::cerr << "[I] found spectral data: " << msd.path() << std::endl;
        fs::file dest = base.child(msd.name());
        if (dest.exists()) {
            std::cerr << "[W] spectral data file already exists. skipping! " << std::endl;
        } else {
            cac.copy(dest);
            std::cerr << "[I] spectral data imported!" << std::endl;
        }
    }

    return 0;
}

static int cmd_import(int argc, char **argv) {
    iris::data::store store = iris::data::store::default_store();

    static struct option longopts[] = {
            { NULL,          0,                      NULL,           0 }
    };

    int ch;
    while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1)
        switch (ch) {
            case '?':
            default:
                std::cerr << "unkown option" << std::endl;
                std::cerr << "usage: import <object>" << std::endl;
                return -1;
        }

    argc -= optind;
    argv += optind;

    if (argc != 1) {
        std::cerr << "usage: import <object>" << std::endl;
        return -1;
    }

    fs::file fd(argv[0]);
    std::string data = fd.read_all();

    YAML::Node doc = YAML::Load(data);
    std::cerr << "[I] objects # in file: " << doc.size() << std::endl;

    for (YAML::const_iterator it = doc.begin(); it !=doc.end(); ++it) {
        std::string entity = it->first.as<std::string>();
        std::cerr << "[I] importing: " << entity << std::endl;
        if (entity == std::string("rgb2lms")) {
            import_rgb2lms(store, fd, data);
        } else {
            std::cerr << "[W] cannot import this object. Skipping!" << std::endl;
        }
    }

    return 0;
}

struct command {
    std::string name;
    std::string help;
    int (*command_fn)(int argc, char **argv);

    int operator()(int argc, char **argv) const {
        return (*command_fn)(argc, argv);
    }
};

command cmds[] = {
        { "info",   "general data store information", cmd_info },
        { "import", "import data [rgb2lms, isoslant, ...] into store ", cmd_import },
        { "",         "", nullptr}
};

static void usage(const std::string &binname = "") {
    std::cerr << "iris data store tool" << std::endl;
    std::cerr << "usage: " << binname << " " << "<command> [<args>]" << std::endl << std::endl;
    std::cerr << "valid commands:" << std::endl;

    command *cmd = nullptr;
    for (cmd = cmds; cmd->command_fn != nullptr; cmd++) {
        std::cerr << "   " << cmd->name << " \t " << cmd->help << std::endl;
    }
}

int main(int argc, char **argv)
{
    static struct option longopts[] = {
            { "help",       no_argument,            NULL,          'h' },
            { NULL,         0,                      NULL,           0 }
    };

    std::string binname = argv[0];

    int ch;
    while ((ch = getopt_long(argc, argv, ":h", longopts, NULL)) != -1)
        switch (ch) {
            case 'h':
                usage(binname);
                return -1;

        }

    argc -= optind;
    argv += optind;

    if (argc == 0) {
        usage(binname);
        return -1;
    }

    std::string cmd_name(argv[0]);

    bool valid_cmd = false;
    command *cmd = nullptr;
    for (cmd = cmds; cmd->command_fn != nullptr; cmd++) {
        if (cmd->name == cmd_name) {
            valid_cmd = true;
            break;
        }
    }

    if (!valid_cmd) {
        std::cerr << "Invalid command: " << cmd_name << std::endl;
        usage(binname);
        return -1;
    }

    int res = 0;
    try {
        res = (*cmd)(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "[E] " << e.what() << std::endl;
        res = -1;
    }
    return res;
}