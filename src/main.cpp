#include <stdio.h>
#include <time.h>

#include "PiTerm.h"



int
main(int argc, char **argv)
{
    #if defined(TERM_GUI)
        SDL_SetMainReady();
    #endif
        
	if (argc <= 1)
	{
		printf("I need a portname (ie /dev/ttyS4)!\n");
		return 0;
	}

	char *port = argv[1];

    int error;
    Interface interface = InterfaceInit(&error, port);
    if (error != 0)
    {
        fprintf(stderr, "Error during initial interface init...\n");
        return 1;
    }

    Term term = TermInit(&error);
    if (error != 0)
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

        if (TermFrameStart(term) != 0)
        {
            running = false;
            break;
        }

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

        TermFrameStop(term);
    }

    InterfaceStop(interface);
    TermStop(term);

	return 0;
}
