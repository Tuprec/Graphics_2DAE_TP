//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef LEAK_DETECTOR_HEADER
#define LEAK_DETECTOR_HEADER

//--- Windows specific includes for memory leak detection to get additional information ---
#if defined(_MSC_VER) && defined(GFX_DEBUG)
    #define _CRTDBG_MAP_ALLOC
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
    #include <Windows.h>
    #include <crtdbg.h>

    // Redefine C allocators to debug versions with file/line info.
    #define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define calloc(c, s) _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define realloc(p, s) _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define free(p) _free_dbg(p, _NORMAL_BLOCK)
#endif

namespace gfx
{
    //--- Class ----
    class LeakDetector final
    {
    public:
        //--- Constructors & Destructor ---
        LeakDetector(const char* write_to_file = "");
        ~LeakDetector() = default;
        LeakDetector(const LeakDetector&) = default;
        LeakDetector& operator=(const LeakDetector&) = default;
        LeakDetector(LeakDetector&&) = default;
        LeakDetector& operator=(LeakDetector&&) = default;

        //--- Functions ---
        static void BreakOnAllocationId(const int id);
        static void CheckForLeaks();
    };
}
#endif //LEAK_DETECTOR_HEADER