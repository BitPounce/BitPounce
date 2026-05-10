#pragma once
#include <BitPounce.h>


struct BoundsInt
{
    glm::ivec3 position;
    glm::ivec3 size;

    BoundsInt() = default;

    BoundsInt(const glm::ivec3& pos, const glm::ivec3& sz)
        : position(pos), size(sz) {}

    glm::ivec3 min() const
    {
        return position;
    }

    glm::ivec3 center() const
    {
        return position + (size / 2);
    }

    glm::ivec3 max() const
    {
        return position + size;
    }

    bool contains(const glm::ivec3& p) const
    {
        glm::ivec3 mn = min();
        glm::ivec3 mx = max();

        return
            p.x >= mn.x && p.x < mx.x &&
            p.y >= mn.y && p.y < mx.y &&
            p.z >= mn.z && p.z < mx.z;
    }
};