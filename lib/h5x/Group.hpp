// Copyright (c) 2013, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#ifndef H5X_GROUP_H
#define H5X_GROUP_H

#include <h5x/LocID.hpp>
#include <h5x/DataSet.hpp>
#include <h5x/DataSpace.hpp>
#include <h5x/Hydra.hpp>

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace h5x {

class Group : public LocID {

public:

    Group() : LocID() {}

    Group(hid_t hid) : LocID(hid) {}

    Group(hid_t hid, bool is_copy) : LocID(hid, is_copy) {}

    Group(const Group &other) : LocID(other) {}

    bool hasObject(const std::string &path) const;
    ndsize_t objectCount() const;
    std::string objectName(ndsize_t index) const;

    bool hasData(const std::string &name) const;

    DataSet createData(const std::string &name, TypeId dtype, const NDSize &size) const;

    DataSet createData(const std::string &name, const DataType &fileType,
            const NDSize &size, const NDSize &maxsize = {}, NDSize chunks = {},
            bool maxSizeUnlimited = true, bool guessChunks = true) const;

    DataSet openData(const std::string &name) const;
    void removeData(const std::string &name);

    template<typename T>
    void setData(const std::string &name, const T &value);
    template<typename T>
    bool getData(const std::string &name, T &value) const;

    bool hasGroup(const std::string &name) const;

    Group openGroup(const std::string &name, bool create = true) const;

    void removeGroup(const std::string &name);
    void renameGroup(const std::string &old_name, const std::string &new_name);

    Group createLink(const Group &target, const std::string &link_name);

    virtual ~Group() { };


private:

    bool objectOfType(const std::string &name, H5O_type_t type) const;

}; // group Group


//template functions

template<typename T>
void Group::setData(const std::string &name, const T &value)
{
    const Hydra<const T> hydra(value);
    TypeId dtype = hydra.element_data_type();
    NDSize shape = hydra.shape();

    DataSet ds;
    if (!hasData(name)) {
        ds = createData(name, dtype, shape);
    } else {
        ds = openData(name);
        ds.setExtent(shape);
    }

    ds.write(dtype, shape, hydra.data());
}


template<typename T>
bool Group::getData(const std::string &name, T &value) const
{
    if (!hasData(name)) {
        return false;
    }

    Hydra<T> hydra(value);
    DataSet ds = openData(name);

    TypeId dtype = hydra.element_data_type();
    NDSize shape = ds.size();

    hydra.resize(shape);
    ds.read(dtype, shape, hydra.data());

    return true;
}

} // h5x


#endif /* NIX_GROUP_H */
