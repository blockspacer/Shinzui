/* Copyright 2016 Guillem Pascual */

#pragma once

#include "defs/common.hpp"

INCL_NOWARN
#include "boost/lockfree/lockfree_forward.hpp"
INCL_WARN

#include <inttypes.h>


#if BUILD_TESTS == ON
    INCL_NOWARN
    #include <boost/lockfree/queue.hpp>
    INCL_WARN

    #include <atomic>

    template <typename T>
    class QueueWithSize
    {
    public:
        explicit QueueWithSize(uint32_t capacity):
            _queue(capacity),
            _counter{ 0 }
        {}

        bool push(T const & t)
        {
            if (_queue.push(t))
            {
                ++_counter;
                return true;
            }
            return false;
        }

        bool pop(T& t)  // NOLINT(runtime/references)
        {
            if (_queue.pop(t))
            {
                --_counter;
                return true;
            }
            return false;
        }

        inline uint32_t size() { return _counter; }

    private:
        boost::lockfree::queue<T> _queue;
        std::atomic<uint32_t> _counter;
    };
#else
    using QueueWithSize<T> = boost::lockfree::queue<T>;
#endif
