#include <stdio.h>
#include <time.h>

#include "PiTerm.h"

#if defined(_WIN32)
    #include "platform_win32.cpp"
    #if defined(TERM_GUI)
        #pragma message("Using IMGUI ui...")
    #else
        #error WIN32 build requires TERM_GUI...
    #endif
#else
    #include "platform_linux.cpp"
    #if defined(TERM_GUI)
        #pragma message("Using IMGUI ui...")
    #else
        #pragma message("Using ncurses ui...")
    #endif
#endif

int
main(int argc, char **argv)
{
	if (argc <= 1)
	{
		printf("I need a portname (ie /dev/ttyS4)!\n");
		return 0;
	}

	char *port = argv[1];

    Interface interface;
    if (InterfaceInit(&interface, port) != 0)
    {
        fprintf(stderr, "Error during initial interface init...\n");
        return 1;
    }

    Term term;
    if (TermInit(&term) != 0)
    {
        fprintf(stderr, "Error during initial terminal init..\n");
        return 1;
    }

#define BUFFER_SIZE (1024)
    u8 buffer[BUFFER_SIZE];

    bool running = true;
    while (running)
    {
        int bytesRead = InterfaceRead(interface, buffer, BUFFER_SIZE);

        if (bytesRead > 0)
        {
            if (TermFrameStart(term) != 0)
            {
                running = false;
                break;
            }

            TermHeaderStart(term);

            time_t t = time(NULL);
            struct tm tm = *localtime(&t);

            int bitsPerSecond = bytesRead * 8;

            TermPrintPos(term, 1, 1, "BAUD: 115200");
            TermPrintPos(term, 2, 1, "RX Rate: %d", bitsPerSecond);

            TermHeaderStop(term);
            TermBodyStart(term);

            TermPrintf(term, "[%d:%d:%d] %.*s",  tm.tm_hour, tm.tm_min, tm.tm_sec, bytesRead, buffer);

            TermBodyStop(term);
            TermFrameStop(term);
        }
    }

    InterfaceStop(interface);
    TermStop(term);

	return 0;
}
