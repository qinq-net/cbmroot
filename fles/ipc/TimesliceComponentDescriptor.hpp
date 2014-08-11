// Copyright 2013 Jan de Cuveland <cmail@cuveland.de>
#pragma once

#ifndef __CINT__
#include <cstdint>
#include <boost/serialization/access.hpp>
#else
#include <stdint.h>
#endif

namespace fles
{

#pragma pack(1)

//! Timeslice component descriptor struct.
struct TimesliceComponentDescriptor
{
    uint64_t ts_num; ///< Timeslice index.
    uint64_t offset; ///< Start offset (in bytes) of corresponding data.
    uint64_t size;   ///< Size (in bytes) of corresponding data.
    uint64_t num_microslices; ///< Number of microslices.

#ifndef __CINT__
    friend class boost::serialization::access;
#endif

    template <class Archive>
    void serialize(Archive& ar, const unsigned int /* version */)
    {
        ar& ts_num;
        ar& offset;
        ar& size;
        ar& num_microslices;
    }
};

#pragma pack()

} // namespace fles {
