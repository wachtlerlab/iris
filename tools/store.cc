

#include <boost/program_options.hpp>
#include <data.h>

#include <getopt.h>

#include <iostream>

void cmd_info(int argc, char **argv) {
    iris::data::store store = iris::data::store::default_store();

    std::cout << "location: " << store.location().path() << std::endl;
    std::cout << "version: " << store.version_string() << std::endl;



}


struct command {
    std::string name;
    std::string help;
    void (*command_fn)(int argc, char **argv);

    void operator()(int argc, char **argv) const {
        (*command_fn)(argc, argv);
    }
};

command cmds[] = {
        { "info", "general data store information", cmd_info },
        {"",         "", nullptr}
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