#if !defined(PI_TERM_PLATFORM_LINUX_H)
#define PI_TERM_PLATFORM_LINUX_H

struct LinuxInterfaceState
{
	int fd;
};

#if !defined(TERM_GUI)
	#include <ncurses.h>

	struct TerminalState
	{
		WINDOW *headerWindow;
		WINDOW *header;

		WINDOW *termWindow;
		WINDOW *term;

		WINDOW *activeWindow;
	};
#endif
#endif
