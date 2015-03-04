// Copyright © 2015 German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Christian Kellner <kellner@bio.lmu.de>

#ifndef NIX_LOCATION_H5_H
#define NIX_LOCATION_H5_H

#include <h5x/HId.hpp>
#include <h5x/Attribute.hpp>
#include <h5x/Hydra.hpp>
#include <h5x/DataTypeHDF5.hpp>

namespace h5x {

class LocID : public HId {
public:
    LocID();

    LocID(hid_t hid);

    LocID(hid_t hid, bool is_copy) : HId(hid, is_copy) { }

    LocID(const LocID &other);

    bool hasAttr(const std::string &name) const;
    void removeAttr(const std::string &name) const;

    template <typename T>
    void setAttr(const std::string &name, const T &value) const;

    template <typename T>
    bool getAttr(const std::string &name, T &value) const;

    void deleteLink(std::string name, hid_t plist = H5L_SAME_LOC);

private:

    Attribute openAttr(const std::string &name) const;
    Attribute createAttr(const std::string &name, h5x::DataType fileType, const DataSpace &fileSpace) const;

};


template<typename T> void LocID::setAttr(const std::string &name, const T &value) const
{
    typedef Hydra<const T> hydra_t;

    const hydra_t hydra(value);
    TypeId dtype = hydra.element_data_type();
    NDSize shape = hydra.shape();

    Attribute attr;

    if (hasAttr(name)) {
        attr = openAttr(name);
    } else {
        DataType fileType = data_type_to_h5_filetype(dtype);
        DataSpace fileSpace = DataSpace::create(shape, false);
        attr = createAttr(name, fileType, fileSpace);
    }

    attr.write(data_type_to_h5_memtype(dtype), shape, hydra.data());
}



template<typename T> bool LocID::getAttr(const std::string &name, T &value) const
{
    if (!hasAttr(name)) {
        return false;
    }

    Hydra<T> hydra(value);

    //determine attr's size and resize value accordingly
    Attribute attr = openAttr(name);
    NDSize dims = attr.extent();
    hydra.resize(dims);

    TypeId dtype = hydra.element_data_type();
    DataType mem_type = data_type_to_h5_memtype(dtype);

    attr.read(mem_type, dims, hydra.data());

    return true;
}

} //h5x::

#endif