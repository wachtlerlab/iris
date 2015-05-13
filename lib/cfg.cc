#include "cfg.h"

iris::cfg::store iris::cfg::store::default_store() {

    fs::file home = fs::file::home_directory();
    fs::file base = home.child(".iris/config");

    if (!base.exists()) {
        throw std::runtime_error("Could not initialize store");
    }

    return cfg::store(home);
}


iris::cfg::store::store(const fs::file &path) : base(path) {

}
