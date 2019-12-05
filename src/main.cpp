#include <stdio.h>
#include <time.h>

#include "PiTerm.h"

#if defined(_WIN32)
    #include "platform_win32.cpp"
    #if defined(TERM_UI)
        #pragma message("Using IMGUI ui...")
    #else
        #error WIN32 build requires TERM_UI...
    #endif
#else
    #include "platform_linux.cpp"
    #if defined(TERM_UI)
        #pragma message("Using IMGUI ui...")
        #error Linux IMGUI not yet supported
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

    TERM term;
    if (TermInit(&term, port) != 0)
    {
        fprintf(stderr, "Error during initial platform terminal configuration...\n");
        return 1;
    }

#define BUFFER_SIZE (1024)
    u8 buffer[BUFFER_SIZE];

    bool running = true;
    while (running)
    {
        int bytesRead = InterfaceRead(term, buffer, BUFFER_SIZE);

        if (bytesRead > 0)
        {
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
        }
    }

    TermStop(term);
	return 0;
}
