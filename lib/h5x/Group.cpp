// Copyright (c) 2013, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#include <h5x/Group.hpp>
#include <h5x/ExceptionHDF5.hpp>


namespace h5x {

bool Group::hasObject(const std::string &name) const {
    // empty string should return false, not exception (which H5Lexists would)
    if (name.empty()) {
        return false;
    }

    HTri res = H5Lexists(hid, name.c_str(), H5P_DEFAULT);
    return res.check("Group::hasObject(): H5Lexists failed");
}

bool Group::objectOfType(const std::string &name, H5O_type_t type) const {
    H5O_info_t info;

    hid_t obj = H5Oopen(hid, name.c_str(), H5P_DEFAULT);

    if (!H5Iis_valid(obj)) {
        return false;
    }

    HErr err = H5Oget_info(obj, &info);
    err.check("Could not obtain object info");

    bool res = info.type == type;

    H5Oclose(obj);
    return res;
}

ndsize_t Group::objectCount() const {
    hsize_t n_objs;
    HErr res = H5Gget_num_objs(hid, &n_objs);
    res.check("Could not get object count");
    return n_objs;
}

std::string Group::objectName(ndsize_t index) const {
    // check if index valid
    if(index > objectCount()) {
		//FIXME: issue #473
        throw std::out_of_range("No object at given index");
    }

    std::string str_name;
    // check whether name is found by index
    ssize_t name_len = H5Lget_name_by_idx(hid,
                                                  ".",
                                                  H5_INDEX_NAME,
                                                  H5_ITER_NATIVE,
                                                  (hsize_t) index,
                                                  NULL,
                                                  0,
                                                  H5P_DEFAULT);
    if (name_len > 0) {
        char* name = new char[name_len+1];
        name_len = H5Lget_name_by_idx(hid,
                                      ".",
                                      H5_INDEX_NAME,
                                      H5_ITER_NATIVE,
                                      (hsize_t) index,
                                      name,
                                      name_len+1,
                                      H5P_DEFAULT);
        str_name = name;
        delete [] name;
    } else {
        throw H5Exception("objectName: No object found, H5Lget_name_by_idx returned no name");
    }

    return str_name;
}


bool Group::hasData(const std::string &name) const {
    return hasObject(name) && objectOfType(name, H5O_TYPE_DATASET);
}


void Group::removeData(const std::string &name) {
    if (hasData(name)) {
        HErr res = H5Gunlink(hid, name.c_str());
        res.check("Group::removeData(): Could not unlink DataSet");
    }
}

DataSet Group::createData(const std::string &name,
                          TypeId dtype,
                          const NDSize &size) const
{
    h5x::DataType fileType = data_type_to_h5_filetype(dtype);
    return createData(name, fileType, size);
}


DataSet Group::createData(const std::string &name,
        const h5x::DataType &fileType,
        const NDSize &size,
        const NDSize &maxsize,
        NDSize chunks,
        bool max_size_unlimited,
        bool guess_chunks) const
{
    DataSpace space;

    if (size) {
        if (maxsize) {
            space = DataSpace::create(size, maxsize);
        } else {
            space = DataSpace::create(size, max_size_unlimited);
        }
    }

    HId dcpl = H5Pcreate(H5P_DATASET_CREATE);
    dcpl.check("Could not create data creation plist");

    if (!chunks && guess_chunks) {
        chunks = DataSet::guessChunking(size, fileType.size());
    }

    if (chunks) {
        int rank = static_cast<int>(chunks.size());
        HErr res = H5Pset_chunk(dcpl.h5id(), rank, chunks.data());
        res.check("Could not set chunk size on data set creation plist");
    }

    DataSet ds = H5Dcreate(hid, name.c_str(), fileType.h5id(), space.h5id(), H5P_DEFAULT, dcpl.h5id(), H5P_DEFAULT);
    ds.check("Group::createData: Could not create DataSet with name " + name);

    return ds;
}


DataSet Group::openData(const std::string &name) const {
    DataSet ds = H5Dopen(hid, name.c_str(), H5P_DEFAULT);
    ds.check("Group::openData(): Could not open DataSet");
    return ds;
}


bool Group::hasGroup(const std::string &name) const {
    return hasObject(name) && objectOfType(name, H5O_TYPE_GROUP);
}


Group Group::openGroup(const std::string &name, bool create) const {
    //check_h5_arg_name(name);

    Group g;

    if (hasGroup(name)) {
        g = Group(H5Gopen(hid, name.c_str(), H5P_DEFAULT));
        g.check("Group::openGroup(): Could not open group: " + name);
    } else if (create) {
        HId gcpl = H5Pcreate(H5P_GROUP_CREATE);
        gcpl.check("Unable to create group with name '" + name + "'! (H5Pcreate)");

        //we want hdf5 to keep track of the order in which links were created so that
        //the order for indexed based accessors is stable cf. issue #387
        HErr res = H5Pset_link_creation_order(gcpl.h5id(), H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED);
        res.check("Unable to create group with name '" + name + "'! (H5Pset_link_cr...)");

        g = Group(H5Gcreate2(hid, name.c_str(), H5P_DEFAULT, gcpl.h5id(), H5P_DEFAULT));
        g.check("Unable to create group with name '" + name + "'! (H5Gcreate2)");

    } else {
        throw H5Exception("Unable to open group with name '" + name + "'!");
    }

    return g;
}

void Group::removeGroup(const std::string &name) {
    if (hasGroup(name))
        H5Gunlink(hid, name.c_str());
}


void Group::renameGroup(const std::string &old_name, const std::string &new_name) {
    //check_h5_arg_name(new_name);

    if (hasGroup(old_name)) {
        H5Gmove(hid, old_name.c_str(), new_name.c_str()); //FIXME: H5Gmove is deprecated
    }
}


Group Group::createLink(const Group &target, const std::string &link_name) {
    //check_h5_arg_name(link_name);

    HErr res = H5Lcreate_hard(target.hid, ".", hid, link_name.c_str(),
                              H5L_SAME_LOC, H5L_SAME_LOC);
    res.check("Unable to create link " + link_name);
    return openGroup(link_name, false);
}



} //h5x