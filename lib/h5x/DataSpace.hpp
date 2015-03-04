// Copyright © 2014, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Christian Kellner <kellner@bio.lmu.de>

#include <h5x/NDSize.hpp>
#include <h5x/HId.hpp>

#ifndef NIX_DATASPACE_H
#define NIX_DATASPACE_H

namespace h5x {

class DataSpace : public HId {
public:

    DataSpace() : HId(H5S_ALL) { }
    DataSpace(hid_t space) : HId(space) { }
    DataSpace(const DataSpace &other) : HId(other) { }

    NDSize extent() const;

    static DataSpace create(const NDSize &dims, const NDSize &maxdims = {});
    static DataSpace create(const NDSize &dims, bool maxdims_unlimited);

};

} //h5x::

#endif
