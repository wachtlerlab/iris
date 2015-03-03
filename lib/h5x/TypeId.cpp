// Copyright © 2014 German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Christian Kellner <kellner@bio.lmu.de>

#include <h5x/TypeId.hpp>

#include <string>
#include <stdexcept>

namespace h5x {

std::string data_type_to_string(TypeId dtype) {

    std::string str;

    switch(dtype) {

    case TypeId::Bool:    str = "Bool";    break;
    case TypeId::Char:    str = "Char";    break;
    case TypeId::Float:   str = "Float";   break;
    case TypeId::Double:  str = "Double";  break;
    case TypeId::Int8:    str = "Int8";    break;
    case TypeId::Int16:   str = "Int16";   break;
    case TypeId::Int32:   str = "Int32";   break;
    case TypeId::Int64:   str = "Int64";   break;
    case TypeId::UInt8:   str = "UInt8";   break;
    case TypeId::UInt16:  str = "UInt16";  break;
    case TypeId::UInt32:  str = "UInt32";  break;
    case TypeId::UInt64:  str = "UInt64";  break;
    case TypeId::String:  str = "String";  break;
    case TypeId::Nothing: str = "Nothing"; break;
    default:
        str = "FIXME";
    }

    return str;
}


std::ostream &operator<<(std::ostream &out, const TypeId dtype) {
    out << data_type_to_string(dtype);
    return out;
}


size_t data_type_to_size(TypeId dtype) {
    switch(dtype) {

        case TypeId::Bool:
            return sizeof(bool);

        case TypeId::Int8:
        case TypeId::UInt8:
            return 1;

        case TypeId::Int16:
        case TypeId::UInt16:
            return 2;

        case TypeId::Int32:
        case TypeId::UInt32:
        case TypeId::Float:
            return 4;

        case TypeId::Int64:
        case TypeId::UInt64:
        case TypeId::Double:
            return 8;

        //strings are char *, but not sure that is the correct thing to do here
        case TypeId::String:
            return sizeof(char *);

        default:
            throw std::invalid_argument("Unkown TypeId");
    }
}

} // h5x::
