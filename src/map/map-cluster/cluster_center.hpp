/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <algorithm>
#include <array>
#include <vector>

#include "defs/common.hpp"
#include "debug/debug.hpp"


class Cluster;
class Cell;

struct ClusterCenter
{
    Cell* center;
    bool mergeSignaled = false;

    constexpr explicit ClusterCenter(Cell* ctr) :
        center(ctr)
    {}
};
