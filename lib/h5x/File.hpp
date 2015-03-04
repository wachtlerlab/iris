// Copyright © 2015 German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Adrian Stoewer <adrian.stoewer@rz.ifi.lmu.de>
//         Christian Kellner <kellner@bio.lmu.de>

#ifndef H5X_H5FILE
#define H5X_H5FILE

#include <h5x/Group.hpp>

#include <string>
#include <boost/optional.hpp>

namespace h5x {
class File : Group {
public:
    File() : Group() {}
    File(hid_t hid) : Group(hid) {}
    File(const File &other) : Group(other) {}

    static File open(const std::string &path, const std::string &mode);

};

} // h5x::

#endif