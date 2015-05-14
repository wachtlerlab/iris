#include "cfg.h"

#include <yaml-cpp/yaml.h>

namespace iris {
namespace cfg {

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


display store::make_display(const monitor       &monitor,
                            const monitor::mode &mode,
                            const std::string   &gfx) const
{

    fs::file linkfile = base.child("links.cfg");
    std::string data = linkfile.read_all();
    YAML::Node root = YAML::Load(data);

    YAML::Node gfx_node = root[gfx];
    YAML::Node link_node = gfx_node[monitor.identifier()];

    std::string link_id = link_node.as<std::string>();

    display dsp;
    dsp.link_id = link_id;
    dsp.monitor_id = monitor.identifier();
    dsp.mode = mode;

    return dsp;
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

static void emit_mode(const monitor::mode &mode, YAML::Emitter &out) {
    out << YAML::BeginMap;
    out << "width" << mode.width;
    out << "height" << mode.height;
    out << "refresh" << mode.refresh;
    out << "color-depth";
    out << YAML::Flow;
    out << YAML::BeginSeq << mode.r << mode.g << mode.b << YAML::EndSeq;

    out << YAML::EndMap; //color-depth
    out << YAML::EndMap; //mode
}

std::string iris::cfg::store::monitor2yaml(const iris::cfg::monitor &monitor) {
    YAML::Emitter out;

    out << YAML::BeginMap;
    out << "monitor";

    out << YAML::BeginMap;
    out << "id" << monitor.identifier();
    out << "name" << monitor.name;
    out << "vendor" << monitor.vendor;
    out << "year" << monitor.year;

    out << "preferred_mode";
    emit_mode(monitor.default_mode, out);
    out << YAML::EndMap; //monitor

    return std::string(out.c_str());
}


std::string store::display2yaml(const display &display) {
    YAML::Emitter out;

    out << YAML::BeginMap;
    out << "display";

    out << YAML::BeginMap;
    out << "monitor_id" << display.monitor_id;
    out << "settings_id" << display.settings_id;
    out << "link_id" << display.link_id;

    out << "gfx" << display.gfx;
    out << "mode";
    emit_mode(display.mode, out);

    out << YAML::EndMap; //
    out << YAML::EndMap; //display

    return std::string(out.c_str());
}


} //iris::cfg::
} //iris::