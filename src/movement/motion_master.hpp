/* Copyright 2016 Guillem Pascual */

#pragma once

#include <utility>

#include "defs/common.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN

class MapAwareEntity;
class MovementGenerator;


enum class MovementFlags
{
    STOPPED =   0x1,
    IDLE =      0x2,
    MOVING =    0x4,
    ROTATING =  0x8
};


class MotionMaster
{
public:
    explicit MotionMaster(MapAwareEntity* owner);

    void teleport(glm::vec3 to);
    inline const glm::vec3& position() { return _position; }
    inline const glm::vec2 position2D() { return { _position.x, _position.z }; }

    void forward(float speed);
    void forward(glm::vec3 forward) { _forward = forward; }
    inline const glm::vec3& forward() { return _forward; }

    void speed(float speed);
    inline const float speed() { return _speed; }

    void move();
    void stop(bool cleanup = false);

    inline bool isMoving() { return (_flags & (uint8_t)MovementFlags::MOVING) == (uint8_t)MovementFlags::MOVING; }
    inline bool isRotating() { return (_flags & (uint8_t)MovementFlags::ROTATING) == (uint8_t)MovementFlags::ROTATING; }

    inline MovementGenerator* generator() { return _generator; }
    void generator(MovementGenerator* generator) { _generator = generator; }
    void update(uint64_t elapsed);

private:
    MapAwareEntity* _owner;
    MovementGenerator* _generator;
    uint8_t _flags;

    glm::vec3 _position;
    glm::vec3 _forward;
    float _rotationAngle;

    float _speed;
};
