// Copyright © 2015 German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Christian Kellner <kellner@bio.lmu.de>

#ifndef H5X_ATTRIBUTE_H
#define H5X_ATTRIBUTE_H

#include <h5x/DataSpace.hpp>
#include <h5x/DataTypeHDF5.hpp>

namespace h5x {

class  Attribute : public HId {
public:
    Attribute();
    Attribute(hid_t hid);
    Attribute(const Attribute &other);

    void read(DataType mem_type, const NDSize &size, void *data);
    void read(DataType mem_type, const NDSize &size, std::string *data);

    void write(DataType mem_type, const NDSize &size, const void *data);
    void write(DataType mem_type, const NDSize &size, const std::string *data);

    DataSpace getSpace() const;
    NDSize extent() const;
};

} // h5x


#endif