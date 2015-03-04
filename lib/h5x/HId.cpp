// Copyright (c) 2013, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#include <h5x/HId.hpp>
#include <h5x/Error.hpp>


namespace h5x {


HId::HId(const HId &other)
    : hid(other.hid)
{
    inc();
}


HId::HId(HId &&other) : hid(other.hid) {
    other.invalidate();
}


HId &HId::operator=(const HId &other) {
    if (hid != other.hid) {
        dec();
        hid = other.hid;
        inc();
    }
    return *this;
}


HId &HId::operator=(HId &&other) {
    hid = other.hid;
    other.invalidate();
    return *this;
}


bool HId::operator==(const HId &other) const {
    if (H5Iis_valid(hid) && H5Iis_valid(other.hid))
        return hid == other.hid;
    else
        return false;
}


bool HId::operator!=(const HId &other) const {
    return !(*this == other);
}


hid_t HId::h5id() const {
    return hid;
}


int HId::refCount() const {
    if (H5Iis_valid(hid)) {
        return H5Iget_ref(hid);
    } else {
        return -1;
    }
}

bool HId::isValid() const {
    HTri res = H5Iis_valid(hid);
    res.check("HId::isValid() failed");
    return res.result();
}

std::string HId::name() const {
    if (! H5Iis_valid(hid)) {
        //maybe throw an exception?
        return "";
    }

    ssize_t len = H5Iget_name(hid, nullptr, 0);

    if (len < 0) {
        throw H5Exception("Could not get size of name");
    }

    std::vector<char> buffer(static_cast<size_t>(len + 1), 0);
    len = H5Iget_name(hid, buffer.data(), buffer.size());

    if (len < 0) {
        throw H5Exception("Could not obtain name");
    }

    std::string name =  std::string(buffer.data());
    return name;
}


void HId::close() {
    dec();
    invalidate();
}


HId::~HId() {
    close();
}


void HId::inc() const {
    if (H5Iis_valid(hid)) {
        H5Iinc_ref(hid);
    }
}


void HId::dec() const {
    if (H5Iis_valid(hid)) {
        H5Idec_ref(hid);
    }
}


void HId::invalidate() {
    hid = H5I_INVALID_HID;
}

} // namespace h5x
