

#include <boost/program_options.hpp>
#include <data.h>

#include <iostream>

void cmd_monitor(const std::vector<std::string> &args) {
    namespace po = boost::program_options;

    po::options_description opts("monitor options");
    opts.add_options()
            ("list", "list installed monitors");

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(args).options(opts).run(), vm);
    } catch (const std::exception &e) {
        std::cerr << "Error while parsing commad line options: " << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
        return;
    }
}


struct command {
    std::string name;
    void (*command_fn)(const std::vector<std::string> &args);

    void operator()(const std::vector<std::string> &args) const {
        (*command_fn)(args);
    }
};

command cmds[] = {
        { "monitor", cmd_monitor },
        {"",         nullptr}
};

int main(int argc, char **argv)
{
    namespace po = boost::program_options;

    po::options_description po_global("configration tool");
    po_global.add_options()
            ("command", po::value<std::string>(), "cmd")
            ("args", po::value<std::vector<std::string> >(), "args for cmd");

    po::positional_options_description pos;
    pos.add("command", 1);
    pos.add("args", -1);

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv)
            .options(po_global)
            .positional(pos)
            .allow_unregistered()
            .run();

    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << po_global << std::endl;
        return 0;
    }

    std::string cmd_name = vm["command"].as<std::string>();

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
        std::cerr << po_global << std::endl;
        return -1;
    }

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    (*cmd)(opts);


    return 0;
}