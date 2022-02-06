//
// Copyright 2020 benmalartre
//
// unlicensed
//
#pragma once

#include "pxr/pxr.h"
#include "pxr/base/vt/array.h"
#include <memory>


/// \struct U2XTopology
///
struct U2XTopology
{
    enum Type {
        POINTS,
        LINES,
        TRIANGLES
    };
    Type                type;
    const int*          samples;
    const int*          bases;
    size_t              numElements;
    size_t              numBases;
};

