#if !defined(PI_TERM_PLATFORM_WIN32_H)
#define PI_TERM_PLATFORM_WIN32_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct WIN32InterfaceState
{
	HANDLE handle;
};

GLOBAL LARGE_INTEGER PerformanceFreq = {};

#endif
