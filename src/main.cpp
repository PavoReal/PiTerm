#include <stdio.h>
#include <time.h>

#pragma warning( push )
#pragma warning( disable : 4100 )
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define STB_SPRINTF_IMPLEMENTATION 
#include "stb_sprintf.h"

#pragma GCC diagnostic pop
#pragma warning( pop )

#include "PiTerm.h"

int
main(int argc, char **argv)
{
    #if defined(TERM_GUI)
        SDL_SetMainReady();
    #endif
        
#define PORT_MAX_LENGTH (KILOBYTES(1))
    char *port = (char*) malloc(PORT_MAX_LENGTH + 1);
	if (argc <= 1)
	{
        strcpy(port, "/dev/ttyUSB0");
	}
    else
    {
        strcpy(port, argv[1]);
    }


    int error;
    Term term = TermInit(&error);

    if (error != 0)
    {
        fprintf(stderr, "Error during initial terminal init..\n");
        return 1;
    }

    Interface interface = InterfaceInit(&error, port);

#define READ_BUFFER_SIZE (1024)
    u8 readBuffer[READ_BUFFER_SIZE];

#define RX_BUFFER_SIZE (MEGABYTES(10))
    u8 *rxBuffer = (u8*) malloc(RX_BUFFER_SIZE);
    u32 rxBufferSize = 0;

    bool interfaceGood = (error == 0);
    bool running = true;

    while (running)
    {
        if (TermFrameStart(term) != 0)
        {
            running = false;
            break;
        }

        TermHeaderStart(term);

        int bytesRead = 0;
        if (interfaceGood)
        {
            bytesRead = InterfaceRead(interface, readBuffer, READ_BUFFER_SIZE);

            TermPrintf(term, "STATUS: Connected -- %s", port);
            TermSameLine(term);
            if (TermButton(term, "Disconnect"))
            {
                InterfaceDisconnect(interface);
                interfaceGood = false;

                time_t t     = time(NULL);
                struct tm tm = *localtime(&t);

                int advance = stbsp_sprintf((char*) (rxBuffer + rxBufferSize), 
                            "[%d:%d:%d] >>> Disconnected to %s <<<\n",  
                            tm.tm_hour, tm.tm_min, tm.tm_sec, port);
                rxBufferSize += advance;
            }
        }
        else
        {
            TermPrintf(term, "STATUS: Disconnected");

            TermPrintf(term, "    ");
            TermSameLine(term);
            if (TermButton(term, "Connect"))
            {
                interfaceGood = (InterfaceReInit(interface, port) == 0);

                time_t t     = time(NULL);
                struct tm tm = *localtime(&t);
                if (interfaceGood)
                {
                    int advance = stbsp_sprintf((char*) (rxBuffer + rxBufferSize), 
                                                "[%d:%d:%d] >>> Connected to %s <<<\n",  
                                                tm.tm_hour, tm.tm_min, tm.tm_sec, port);
                    rxBufferSize += advance;
                }
                else
                {
                    int advance = stbsp_sprintf((char*) (rxBuffer + rxBufferSize), 
                                                "[%d:%d:%d] >>> Could not connect to %s <<<\n",  
                                                tm.tm_hour, tm.tm_min, tm.tm_sec, port);
                    rxBufferSize += advance;
                }
            }
            TermSameLine(term);
            TermInputText(term, "Port", port, PORT_MAX_LENGTH);
        }

        int bitsPerSecond = bytesRead * 8;

        TermPrintf(term, "BAUD: 115200");
        TermPrintf(term, "RX Rate: %d", bitsPerSecond);
        TermPrintf(term, "RX Buffer Size: %d / %d", rxBufferSize, RX_BUFFER_SIZE);

        TermHeaderStop(term);
        TermBodyStart(term);

        TermPrintBuffer(term, rxBuffer, rxBufferSize);

        if (bytesRead)
        {
            time_t t     = time(NULL);
            struct tm tm = *localtime(&t);

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
    free(port);

	return 0;
}
