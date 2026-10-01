//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <leak_detector.h>
using namespace gfx;

#if defined(_MSC_VER) && defined(GFX_DEBUG)
static const char* g_leaks_report_file = nullptr;
LeakDetector::LeakDetector(const char* write_to_file)
{
	// If write_to_file is not empty, set the output file for memory leaks.
	// Because this writing might be triggered by the CRT, and thus the OS,
	// we need to ensure that the file is opened correctly. Because of this,
	// we do need to rely on OS specific code (ofsteam or FILE) might be invalid
	// at the end of the program!
	if (write_to_file && write_to_file[0] != '\0')
	{
		g_leaks_report_file = write_to_file;
		const DWORD dw_file_attr = GetFileAttributes(g_leaks_report_file);
		if (dw_file_attr != INVALID_FILE_ATTRIBUTES && !(dw_file_attr & FILE_ATTRIBUTE_DIRECTORY))
			DeleteFile(g_leaks_report_file);
		const auto custom_crt_report_hook = [](int report_type, char* msg, int* return_val) -> int
			{
				(void)report_type; (void)return_val;
				HANDLE h_leaks_report_file = CreateFile(g_leaks_report_file,
					GENERIC_WRITE, FILE_SHARE_READ,
					nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (h_leaks_report_file == INVALID_HANDLE_VALUE)
					return 1;
				SetFilePointer(h_leaks_report_file, 0, nullptr, FILE_END);
				DWORD dw_bytes_written = 0;
				WriteFile(h_leaks_report_file, msg, static_cast<DWORD>(strlen(msg)), &dw_bytes_written, nullptr);
				CloseHandle(h_leaks_report_file); // Always close in between calls. Not ideal but necessary to keep it to this single function.
				h_leaks_report_file = INVALID_HANDLE_VALUE;
				return 0; // Returning 0 to indicate the report has been handled.
			};
		_CrtSetReportHook(custom_crt_report_hook);
	}

	// Enable debug heap allocations and automatic leak check at program exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

void LeakDetector::BreakOnAllocationId(const int id)
{
	_crtBreakAlloc = id;
}

void LeakDetector::CheckForLeaks()
{
	_CrtDumpMemoryLeaks();
}

#else

LeakDetector::LeakDetector(const char*) {}
void LeakDetector::BreakOnAllocationId(const int) {}
void LeakDetector::CheckForLeaks() {}

#endif