#include <stdio.h>
#include <time.h>

#define STB_SPRINTF_IMPLEMENTATION 
#include "stb_sprintf.h"

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

#define READ_BUFFER_SIZE (1024)
    u8 readBuffer[READ_BUFFER_SIZE];

#define RX_BUFFER_SIZE (MEGABYTES(1))
    u8 *rxBuffer = (u8*) malloc(RX_BUFFER_SIZE);
    u32 rxBufferSize = 0;

    bool running = true;
    while (running)
    {
        int bytesRead = InterfaceRead(interface, readBuffer, READ_BUFFER_SIZE);

        if (TermFrameStart(term) != 0)
        {
            running = false;
            break;
        }

        TermHeaderStart(term);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        int bitsPerSecond = bytesRead * 8;

        TermPrintf(term, "BAUD: 115200");
        TermPrintf(term, "RX Rate: %d", bitsPerSecond);
        TermPrintf(term, "RX Buffer Size: %d / %d", rxBufferSize, RX_BUFFER_SIZE);

        TermHeaderStop(term);
        TermBodyStart(term);

        TermPrintBuffer(term, rxBuffer, rxBufferSize);

        if (bytesRead)
        {
             int advance = stbsp_sprintf((char*) (rxBuffer + rxBufferSize), 
                                         "[%d:%d:%d] %.*s\r\n",  
                                         tm.tm_hour, tm.tm_min, tm.tm_sec, 
                                         bytesRead, readBuffer);

             rxBufferSize += advance;
        }

        TermBodyStop(term);
        TermFrameStop(term);
    }

    InterfaceStop(interface);
    TermStop(term);

    free(rxBuffer);

	return 0;
}
