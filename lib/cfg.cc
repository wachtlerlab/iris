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


rgb2lms store::load_rgb2lms(const display &display) const {

    // need to find the latest calibration that fits the display

    fs::file mdir = base.child(display.monitor_id);
    std::cout << mdir.path() << std::endl;

    std::vector<fs::file> res;
    std::copy_if(mdir.children().begin(), mdir.children().end(),
              std::back_inserter(res), fs::fn_matcher("*.rgb2sml"));

    std::sort(res.begin(), res.end(), [](const fs::file &a, const fs::file &b) {
        return a.name() > b.name();
    });

    for (const fs::file f : res) {
        std::string data = f.read_all();
        rgb2lms ca = yaml2rgb2lms(data);

        const struct display &d = ca.dsy;

        if (d.monitor_id      == display.monitor_id &&
                d.settings_id == display.settings_id &&
                d.link_id     == display.link_id &&
                d.gfx         == d.gfx) {

            //FIXME:: check mode too
            return ca;
        }
    }

    throw std::runtime_error("Could not find any matching rgb2lms matrix");
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

static display yaml2display(const YAML::Node &root) {

    display display;
    display.gfx = root["gfx"].as<std::string>();
    display.link_id = root["link_id"].as<std::string>();
    display.monitor_id = root["monitor_id"].as<std::string>();
    display.settings_id = root["settings_id"].as<std::string>();
    display.mode = yaml2mode(root["mode"]);

    return display;

}

static std::pair<float, float> yaml2size(const YAML::Node &root) {
    float width = root["width"].as<float>();
    float height = root["height"].as<float>();
    return std::make_pair(width, height);
}

rgb2lms store::yaml2rgb2lms(const std::string &data) {
    YAML::Node doc = YAML::Load(data);
    YAML::Node root = doc["rgb2lms"];

    rgb2lms calibration(root["id"].as<std::string>());
    std::tie(calibration.width, calibration.height) = yaml2size(root["size"]);
    calibration.dsy = yaml2display(root["display"]);

    std::string csv = root["dkl"].as<std::string>();
    calibration.dkl_params = dkl::parameter::from_csv_data(csv);

    return calibration;
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