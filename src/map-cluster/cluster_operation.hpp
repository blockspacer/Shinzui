#pragma once

#include "offset.hpp"

#include <inttypes.h>


class Cell;
class ClusterCenter;

enum class ClusterOperationType
{
    MERGE,
    RING_INVALIDATION
};

struct ClusterOperation
{
    ClusterOperationType type;
    ClusterCenter* cluster1;
    ClusterCenter* cluster2;
    int32_t radius;
};
