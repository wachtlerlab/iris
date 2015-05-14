#include "cfg.h"

#include <yaml-cpp/yaml.h>

iris::cfg::store iris::cfg::store::default_store() {

    fs::file home = fs::file::home_directory();
    fs::file base = home.child(".iris/config");

    if (!base.exists()) {
        throw std::runtime_error("Could not initialize store");
    }

    return cfg::store(base);
}


iris::cfg::store::store(const fs::file &path) : base(path) {

}

std::string iris::cfg::store::default_monitor() const {

    fs::file dm_link = base.child("default.monitor");

    if (!dm_link.exists()) {
        throw std::runtime_error("cfg: no default monitor found @ " + dm_link.path());
    }

    fs::file target = dm_link.readlink();
    return target.name();
}

iris::cfg::monitor iris::cfg::store::load_monitor(const std::string &uid) const {
    fs::file mfs = base.child(uid + "/" + uid + ".monitor");
    return yaml2monitor(mfs.read_all());
}


// yaml stuff

static iris::cfg::monitor::mode yaml2mode(const YAML::Node &node) {
    iris::cfg::monitor::mode mode;

    mode.width = node["width"].as<float>();
    mode.height = node["height"].as<float>();
    mode.refresh = node["refresh"].as<float>();

    YAML::Node depth = node["color-depth"];
    mode.r = depth[0].as<int>();
    mode.g = depth[1].as<int>();
    mode.b = depth[2].as<int>();

    return mode;
}

iris::cfg::monitor iris::cfg::store::yaml2monitor(const std::string &data) {
    std::cerr << data << std::endl;

    YAML::Node root = YAML::Load(data);
    std::cerr << root.Type() << std::endl;

    YAML::Node start = root["monitor"];
    std::cerr << start.Type() << std::endl;

    monitor monitor(start["id"].as<std::string>());

    monitor.name = start["name"].as<std::string>();
    monitor.vendor = start["vendor"].as<std::string>();
    monitor.year = start["year"].as<std::string>();

    monitor.default_mode = yaml2mode(start["preferred_mode"]);

    return monitor;
}
