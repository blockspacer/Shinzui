/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <vector>
#include <glm/glm.hpp>

#include "map/map_aware_entity.hpp"


template <int MaxEntities, int MaxDepth>
class QuadTree
{
public:
    void insert(MapAwareEntity* entity);

protected:
    QuadTree();
    QuadTree(int depth, glm::vec4 bounds);

    std::vector<QuadTree<MaxEntities, MaxDepth>*> _nodes;
    std::vector<MapAwareEntity*> _entities;

    int getIndex(glm::vec4 rect);
    void split();
    void clear();

private:
    int _depth;
    glm::vec4 _bounds; // (x0, y0, x1, y1) ~ (x, y, z, t)
};

template <int MaxEntities, int MaxDepth>
class RadialQuadTree : public QuadTree<MaxEntities, MaxDepth>
{
public:
    RadialQuadTree(glm::vec2 center, float radius);

private:
    glm::vec2 _center;
    float _radius;
};


template <int MaxEntities, int MaxDepth>
QuadTree<MaxEntities, MaxDepth>::QuadTree() :
    _depth(0)
{}

template <int MaxEntities, int MaxDepth>
QuadTree<MaxEntities, MaxDepth>::QuadTree(int depth, glm::vec4 bounds) :
    _depth(depth),
    _bounds(bounds)
{}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::insert(MapAwareEntity* entity)
{
    if (!_nodes.empty())
    {
        int index = getIndex(entity->boundingBox()->asRect());
        if (index != -1)
        {
            _nodes[index]->insert(entity);
            return;
        }
    }

    _entities.push_back(entity);

    if (_entities.size() > MaxEntities && _depth < MaxDepth)
    {
        if (_nodes.empty())
        {
            split();
        }

        for (auto it = _entities.begin(); it != _entities.end();)
        {
            auto entity = *it;
            
            int index = getIndex(entity->boundingBox()->asRect());
            if (index != -1)
            {
                _nodes[index]->insert(entity);
                it = _entities.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

template <int MaxEntities, int MaxDepth>
int QuadTree<MaxEntities, MaxDepth>::getIndex(glm::vec4 rect)
{
    float verticalMidpoint = _bounds.x + ((_bounds.z - _bounds.x) / 2);
    float horizontalMidpoint = _bounds.y + ((_bounds.t - _bounds.y) / 2);

    // Object can completely fit within the top quadrants
    bool topQuadrant = (rect.y < horizontalMidpoint && rect.y + (rect.t - rect.y) < horizontalMidpoint);
    // Object can completely fit within the bottom quadrants
    bool bottomQuadrant = (rect.y > horizontalMidpoint);

    // Object can completely fit within the left quadrants
    if (rect.x < verticalMidpoint && rect.x + (rect.z - rect.x) < verticalMidpoint)
    {
        if (topQuadrant) return 1;
        if (bottomQuadrant) return 2;
    }
    // Object can completely fit within the right quadrants
    else if (rect.x > verticalMidpoint)
    {
        if (topQuadrant) return 0;
        if (bottomQuadrant) return 3;
    }

    return -1;
}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::split()
{
    int subWidth = static_cast<int>((_bounds.z - _bounds.x) / 2);
    int subHeight = static_cast<int>((_bounds.t - _bounds.y) / 2);
    int x = static_cast<int>(_bounds.x);
    int y = static_cast<int>(_bounds.y);

    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x + subWidth, y, subWidth, subHeight }));
    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x, y, subWidth, subHeight }));
    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x, y + subHeight, subWidth, subHeight }));
    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x + subWidth, y + subHeight, subWidth, subHeight }));
}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::clear()
{
    _entities.clear();

    for (auto*& node : _nodes)
    {
        node->clear();
        delete node;
    }

    _nodes.clear();
}

template <int MaxEntities, int MaxDepth>
RadialQuadTree<MaxEntities, MaxDepth>::RadialQuadTree(glm::vec2 center, float radius) :
    QuadTree<MaxEntities, MaxDepth>(),
    _center(center),
    _radius(radius)
{};