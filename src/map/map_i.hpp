#include "map.hpp"
#include "cell.hpp"
#include "debug.hpp"


template <typename E>
Map<E>::Map(int32_t x, int32_t y, uint32_t dx, uint32_t dy) :
    _x(x), _y(y),
    _dx(dx), _dy(dy)
{}

template <typename E>
Cell<E>* Map<E>::addTo2D(int16_t x, int16_t y, E e)
{
    return addTo(offsetOf(x, y), e);
}

template <typename E>
Cell<E>* Map<E>::addTo(int16_t q, int16_t r, E e)
{
    return addTo(Offset(q, r), e);
}

template <typename E>
Cell<E>* Map<E>::addTo(const Offset&& offset, E e)
{
    auto cell = getOrCreate(std::move(offset), true);
    // cell->add(e);
    return cell;
}

template <typename E>
Cell<E>* Map<E>::get(int16_t q, int16_t r)
{
    return get(Offset(q, r));
}

template <typename E>
Cell<E>* Map<E>::get(const Offset& offset)
{
    auto it = _cells.find(offset.hash());

    if (it == _cells.end())
    {
        return nullptr;
    }

    return (*it).second;
}

template <typename E>
Cell<E>* Map<E>::getOrCreate(int16_t q, int16_t r, bool siblings)
{
    return getOrCreate(Offset(q, r), siblings);
}

template <typename E>
Cell<E>* Map<E>::getOrCreate(const Offset& offset, bool siblings)
{
    auto cell = get(offset);

    if (!cell)
    {
        auto result = _cells.emplace(offset.hash(), new Cell<E>(this, std::move(offset)));
        cell = (*result.first).second;
    }

    if (siblings)
    {
        LOG(LOG_CELLS, "Creating siblings");
        auto siblings = createSiblings(cell);
    }

    return cell;
}

template <typename E>
std::vector<Cell<E>*> Map<E>::createSiblings(Cell<E>* cell)
{
    const Offset& offset = cell->offset();
    int16_t q = offset.q();
    int16_t r = offset.r();

    return {
        getOrCreate(q + 0, r - 1, false),
        getOrCreate(q + 1, r - 1, false),
        getOrCreate(q + 1, r + 0, false),
        getOrCreate(q + 0, r + 1, false),
        getOrCreate(q - 1, r + 1, false),
        getOrCreate(q - 1, r + 0, false),
    };
}
