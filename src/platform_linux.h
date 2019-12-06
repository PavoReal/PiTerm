#if !defined(PI_TERM_PLATFORM_LINUX_H)
#define PI_TERM_PLATFORM_LINUX_H

struct LinuxInterfaceState
{
	int fd;
};

#if defined(TERM_GUI)
	#include "imgui.h"

	struct LinuxTerminalState
	{
		LinuxInterfaceState interface;
	};
#else
	#include <ncurses.h>

	struct LinuxTerminalState
	{
		LinuxInterfaceState interface;

		WINDOW *headerWindow;
		WINDOW *header;

		WINDOW *termWindow;
		WINDOW *term;

		WINDOW *activeWindow;
	};
#endif
#endif
