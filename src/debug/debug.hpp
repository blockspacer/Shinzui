/* Copyright 2016 Guillem Pascual */

#pragma once

// For abort and printf
#include <stdlib.h>
#include <stdio.h>

#define LOG_NOTHING             0x0000000000000000
#define LOG_ALL                 0xFFFFFFFFFFFFFFFF
#define LOG_DEBUG               0x0000000000000001
#define LOG_CELLS               0x0000000000000002
#define LOG_CLUSTERS            0x0000000000000004
#define LOG_FATAL               0x0000000000000008
#define LOG_CLIENT_LIFECYCLE    0x0000000000000010
#define LOG_PACKET_LIFECYCLE    0x0000000000000020
#define LOG_PACKET_RECV         0x0000000000000040
#define LOG_PACKET_SEND         0x0000000000000080
#define LOG_SERVER_LOOP         0x0000000000000100
#define LOG_MOVEMENT_GENERATOR  0x0000000000000200
#define LOG_CELL_CHANGES        0x0000000000000400
#define LOG_FIRE_LOGIC          0x0000000000000800
#define LOG_FIRE_LOGIC_EXT      0x0000000000001000
#define LOG_QUADTREE            0x0000000000002000
#define LOG_SPAWNS              0x0000000000004000
#define LEADING_ZEROS           4

// #define LOG_LEVEL               LOG_CLIENT_LIFECYCLE | LOG_PACKET_LIFECYCLE | LOG_PACKET_RECV | LOG_PACKET_SEND | LOG_FIRE_LOGIC | LOG_FIRE_LOGIC_EXT  // NOLINT
#define LOG_LEVEL               LOG_ALL & ~LOG_SERVER_LOOP

#define STR(a)                  STR_(a)
#define STR_(a)                 #a

// Expand is a trick for MSVC __VA_ARGS__ to work :(
#define EXPAND(x)               x

#ifndef FUNCTION_NAME
    #if defined(_MSC_VER)
        #define FUNCTION_NAME __FUNCTION__
        #define FILE_NAME __FILE__ + BASE_DIR_LEN
    #else
        #define FILE_NAME __FILE__ + BASE_DIR_LEN

        static inline void __dummy_func__()
        {
            #ifdef __PRETTY_FUNCTION__
                #define FUNCTION_NAME __PRETTY_FUNCTION__
            #elif __FUNCTION__
                #define FUNCTION_NAME __FUNCTION__
            #else
                #define FUNCTION_NAME __func__
            #endif
        }
    #endif
#endif

template <typename T>
bool nop(T value)
{
    (void)value;
    return true;
}

template <typename First, typename... Rest>
bool nop(First firstValue, Rest... rest)
{
    nop(firstValue);
    return nop(rest...);
}

#if defined(FORCE_ASCII_DEBUG)
    #define LOG_FMT(fmt) "\x1B[01;44m[%." STR(LEADING_ZEROS) "X] %s:" STR(__LINE__) "(%s) \x1B[00m\x1B[0;34;49m\xee\x82\xb0\x1B[00m" fmt "\n%s"
    #define LOG_FNC printf
    #define ASSERT_FNC printf
#else
    #include "debug/reactive.hpp"

    #define LOG_FMT(fmt) "  \x1B[01;44m[%." STR(LEADING_ZEROS) "X] %s:" STR(__LINE__) "(%s) \x1B[00m\x1B[0;34;49m\xee\x82\xb0\x1B[00m" fmt "\n%s"
    #define LOG_FNC Reactive::get()->print
    #define ASSERT_FNC Reactive::get()->print_now
#endif

#define EXPAND_HELPER(fnc, lvl, fmt, ...) \
    EXPAND(fnc(LOG_FMT(fmt), lvl, FILE_NAME, FUNCTION_NAME, __VA_ARGS__))  // NOLINT(whitespace/line_length)

#define LOG_ALWAYS(...)             EXPAND(EXPAND_HELPER(printf, -1, __VA_ARGS__, ""))

#define FORCE_DEBUG

#if defined(FORCE_DEBUG) || ((!defined(NDEBUG) || defined(_DEBUG)) && BUILD_TESTS != ON)
    #define IF_LOG(lvl)         (lvl & (LOG_LEVEL))  // NOLINT
    #define LOG(lvl, ...)       (((lvl & (LOG_LEVEL)) && EXPAND(EXPAND_HELPER(LOG_FNC, lvl, __VA_ARGS__, ""))) || ((lvl & ~(LOG_LEVEL)) && EXPAND(EXPAND_HELPER(nop, lvl, __VA_ARGS__, ""))))  // NOLINT
#else
    #define IF_LOG(lvl)         false
    #define LOG(lvl, ...)       ((void)(0))
    // #define ASSERT(expr, ...)   ((void)(0))
#endif

#define LOG_ASSERT(expr, ...) ((expr) ? (void)(0) : (EXPAND_HELPER(ASSERT_FNC, -1, __VA_ARGS__, ""), abort()))
