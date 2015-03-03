// Copyright © 2014, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Christian Kellner <kellner@bio.lmu.de>

#ifndef NIX_DATATYPE_H
#define NIX_DATATYPE_H


#include <cstdint>
#include <ostream>

namespace h5x {

/**
 * @brief Enumeration providing constants for all valid data types.
 *
 * Those data types are used by {@link nix::DataArray} and {@link nix::Property}
 * in order to indicate of what type the stored data of value is.
 */
enum class TypeId {
    Bool,
    Char,
    Float,
    Double,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    String,
    Date,
    DateTime,
    Opaque,

    Nothing = -1
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<typename T>
struct to_type_id {
    static const bool is_valid = false;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<bool> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Bool;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<char> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Char;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<float> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Float;
};

/**
 * @brief Determine if a type is a valid data type.
 *
 * @internal
 */
template<>
struct to_type_id<double> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Double;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<int8_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Int8;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<uint8_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::UInt8;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<int16_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Int16;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<uint16_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::UInt16;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<int32_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Int32;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<uint32_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::UInt32;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<int64_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::Int64;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<uint64_t> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::UInt64;
};

/**
 * @brief Determine if a type is a valid data type.
 */
template<>
struct to_type_id<std::string> {
    static const bool is_valid = true;
    static const TypeId value = TypeId::String;
};

/**
 * @brief Determine the size of a data type.
 *
 * @param dtype         The data type.
 *
 * @return The size of the type.
 */
size_t data_type_to_size(TypeId dtype);

/**
 * @brief Convert a data type into string representation.
 *
 * @param dtype         The data type.
 *
 * @return A human readable name for the given type.
 */
std::string data_type_to_string(TypeId dtype);

/**
 * @brief Output operator for data type.
 *
 * Prints a human readable string representation of the
 * data type to an output stream.
 *
 * @param out           The output stream.
 * @param dtype         The data type to print.
 *
 * @return The output stream.
 */
std::ostream& operator<<(std::ostream &out, const TypeId dtype);


} // h5x::

#endif
