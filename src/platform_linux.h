#if !defined(PI_TERM_PLATFORM_LINUX_H)
#define PI_TERM_PLATFORM_LINUX_H

#include <ncurses.h>

struct LinuxTerminalState
{
	int fd;

	WINDOW *headerWindow;
	WINDOW *header;

	WINDOW *termWindow;
	WINDOW *term;

	WINDOW *activeWindow;
};

#endif
