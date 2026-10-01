//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef DEBUG_HEADER
#define DEBUG_HEADER

//--- Helper Macro ---
#include <csignal>
#if defined(_MSC_VER)
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#define DEBUG_BREAK() ((void)0) // Fallback that does nothing.
#endif

#define BREAK_IF_MATCH(a, b, val1, val2)   \
do {                                       \
    if ((uint32_t)(a) == (uint32_t)(val1)  \
     && (uint32_t)(b) == (uint32_t)(val2)) \
    {                                      \
        DEBUG_BREAK();                     \
    }                                      \
} while (0)

#endif //DEBUG_HEADER