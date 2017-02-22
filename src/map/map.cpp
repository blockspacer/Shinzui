/* Copyright 2016 Guillem Pascual */

#include "map.hpp"
#include "cell.hpp"
#include "debug.hpp"
#include "cluster.hpp"
#include "map_operation.hpp"
#include "map_aware_entity.hpp"
#include "motion_master.hpp"

#include <algorithm>
#include <iterator>
#include <list>
#include <utility>
#include <vector>
#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>


Map::Map(boost::object_pool<Cell>* cellAllocator)
{
    _cellAllocator = cellAllocator ? cellAllocator : new boost::object_pool<Cell>(2048);
    _cluster = new Cluster();
    _scheduledOperations = new boost::lockfree::queue<MapOperation*>(2048);
}

Map::~Map()
{
    delete _cellAllocator;
    delete _cluster;
    delete _scheduledOperations;
}

void Map::update(uint64_t elapsed)
{
    runScheduledOperations();
    cluster()->update(elapsed);
    cluster()->runScheduledOperations();
}

void Map::runScheduledOperations()
{
    MapOperation* operation;
    while (_scheduledOperations->pop(operation))
    {
        Cell* cell = nullptr;

        switch (operation->type)
        {
            case MapOperationType::ADD_ENTITY:
                cell = getOrCreate(std::move(operation->offset));
                if (cell)
                {
                    operation->entity->cell(cell);

                    if (operation->entity->client())
                    {
                        cell->_playerData.emplace(operation->entity->id(), operation->entity);
                        cluster()->add(operation->entity, createSiblings(cell));
                    }
                    else
                    {
                        cell->_data.emplace(operation->entity->id(), operation->entity);
                    }

                    operation->entity->onAdded(cell, operation->param);
                }
                break;

            case MapOperationType::REMOVE_ENTITY:
                cell = get(std::move(operation->offset));
                if (cell)
                {
                    if (operation->entity->client())
                    {
                        cell->_playerData.erase(operation->entity->id());
                        cluster()->remove(operation->entity);
                    }
                    else
                    {
                        cell->_data.erase(operation->entity->id());
                    }

                    operation->entity->onRemoved(cell, operation->param);
                }
                break;

            case MapOperationType::DESTROY:
                // TODO(gpascualg): Not implemented yet
                break;

            default:
                // TODO(gpascualg): Unkown operation error
                break;
        }

        delete operation;
    }
}

void Map::broadcastToSiblings(Cell* cell, boost::intrusive_ptr<Packet> packet)
{
    cell->broadcast(packet);

    auto cells = getSiblings(cell);
    broadcast(cells, packet);
}

void Map::broadcastExcluding(Cell* cell, Cell* exclude, boost::intrusive_ptr<Packet> packet)
{
    auto cells = getCellsExcluding(cell, exclude);
    broadcast(cells, packet);
}

std::list<Cell*> Map::getCellsExcluding(Cell* cell, Cell* exclude)
{
    auto offsetCell = cell->offset();
    auto offsetExclude = exclude->offset();
    auto directionQ = offsetCell.q() - offsetExclude.q();
    auto directionR = offsetCell.r() - offsetExclude.r();

    auto idx = directionIdxs(directionQ, directionR);
    auto i = (idx - 1) % MAX_DIR_IDX;
    auto j = (idx + 1) % MAX_DIR_IDX;

    auto cell1 = getOrCreate({ offsetCell.q() + directionQ + directionQ,  // NOLINT(whitespace/braces)
        offsetCell.r() + directionR + directionR });  // NOLINT(whitespace/braces)

    auto cell2 = getOrCreate({ offsetCell.q() + directionQ + directions[i].q,  // NOLINT(whitespace/braces)
        offsetCell.r() + directionR + directions[i].r });  // NOLINT(whitespace/braces)

    auto cell3 = getOrCreate({ offsetCell.q() + directionQ + directions[j].q,  // NOLINT(whitespace/braces)
        offsetCell.r() + directionR + directions[j].r });  // NOLINT(whitespace/braces)

    return { cell1, cell2, cell3 };
}

void Map::onMove(MapAwareEntity* entity)
{
    auto& pos = entity->motionMaster()->position();
    Cell* cell = getOrCreate(offsetOf(pos.x, pos.y));
    if (cell != entity->cell())
    {
        removeFrom(entity->cell(), entity, cell);
        addTo(cell, entity, cell);
    }
}

void Map::addTo(MapAwareEntity* e, Cell* old)
{
    addTo2D(e->motionMaster()->position(), e, old);
}

void Map::addTo2D(const glm::vec2& pos, MapAwareEntity* e, Cell* old)
{
    addTo(offsetOf(pos.x, pos.y), e, old);
}

void Map::addTo(int32_t q, int32_t r, MapAwareEntity* e, Cell* old)
{
    addTo(Offset(q, r), e, old);
}

void Map::addTo(const Offset&& offset, MapAwareEntity* e, Cell* old)
{
    _scheduledOperations->push(new MapOperation {  // NOLINT(whitespace/braces)
        MapOperationType::ADD_ENTITY,
        offset,
        e,
        old
    });  // NOLINT(whitespace/braces)
}

void Map::addTo(Cell* cell, MapAwareEntity* e, Cell* old)
{
    _scheduledOperations->push(new MapOperation{  // NOLINT(whitespace/braces)
        MapOperationType::ADD_ENTITY,
        cell->offset(),
        e,
        old
    });  // NOLINT(whitespace/braces)
}

void Map::removeFrom(MapAwareEntity* e, Cell* to)
{
    removeFrom2D(e->motionMaster()->position(), e, to);
}

void Map::removeFrom2D(const glm::vec2& pos, MapAwareEntity* e, Cell* to)
{
    removeFrom(offsetOf(pos.x, pos.y), e, to);
}

void Map::removeFrom(int32_t q, int32_t r, MapAwareEntity* e, Cell* to)
{
    removeFrom(Offset(q, r), e, to);
}

void Map::removeFrom(const Offset&& offset, MapAwareEntity* e, Cell* to)
{
    _scheduledOperations->push(new MapOperation{  // NOLINT(whitespace/braces)
        MapOperationType::REMOVE_ENTITY,
        offset,
        e,
        to
    });  // NOLINT(whitespace/braces)
}

void Map::removeFrom(Cell* cell, MapAwareEntity* e, Cell* to)
{
    _scheduledOperations->push(new MapOperation{  // NOLINT(whitespace/braces)
        MapOperationType::REMOVE_ENTITY,
        cell->offset(),
        e,
        to
    });  // NOLINT(whitespace/braces)
}

Cell* Map::get(int32_t q, int32_t r)
{
    return get(Offset(q, r));
}

Cell* Map::get(const Offset& offset)
{
    auto it = _cells.find(offset.hash());

    if (it == _cells.end())
    {
        return nullptr;
    }

    return (*it).second;
}

Cell* Map::getOrCreate(int32_t q, int32_t r)
{
    return getOrCreate(Offset(q, r));
}

Cell* Map::getOrCreate(const Offset& offset)
{
    auto cell = get(offset);
    if (!cell)
    {
        // Allocate cell
        auto result = _cells.emplace(offset.hash(), _cellAllocator->construct(this, offset));
        cell = (*result.first).second;
    }

    return cell;
}

std::vector<Cell*> Map::createSiblings(Cell* cell)
{
    const Offset& offset = cell->offset();
    int32_t q = offset.q();
    int32_t r = offset.r();

    return {  // NOLINT(whitespace/braces)
        getOrCreate(q + 0, r - 1),
        getOrCreate(q + 1, r - 1),
        getOrCreate(q + 1, r + 0),
        getOrCreate(q + 0, r + 1),
        getOrCreate(q - 1, r + 1),
        getOrCreate(q - 1, r + 0),
    };
}

std::vector<Cell*> Map::getSiblings(Cell* cell)
{
    const Offset& offset = cell->offset();
    int32_t q = offset.q();
    int32_t r = offset.r();

    return{  // NOLINT(whitespace/braces)
        get(q + 0, r - 1),
        get(q + 1, r - 1),
        get(q + 1, r + 0),
        get(q + 0, r + 1),
        get(q - 1, r + 1),
        get(q - 1, r + 0),
    };
}
