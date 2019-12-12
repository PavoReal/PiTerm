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
    u8 txBuffer[READ_BUFFER_SIZE] = {};

#define CONSOLE_BUFFER_SIZE (MEGABYTES(10))
    u8 *consoleBuffer = (u8*) malloc(CONSOLE_BUFFER_SIZE);
    u32 consoleBufferSize = 0;

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

                int advance = stbsp_sprintf((char*) (consoleBuffer + consoleBufferSize), 
                            "[%d:%d:%d] >>> Disconnected to %s <<<\n",  
                            tm.tm_hour, tm.tm_min, tm.tm_sec, port);
                consoleBufferSize += advance;
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
                    int advance = stbsp_sprintf((char*) (consoleBuffer + consoleBufferSize), 
                                                "[%d:%d:%d] >>> Connected to %s <<<\n",  
                                                tm.tm_hour, tm.tm_min, tm.tm_sec, port);
                    consoleBufferSize += advance;
                }
                else
                {
                    int advance = stbsp_sprintf((char*) (consoleBuffer + consoleBufferSize), 
                                                "[%d:%d:%d] >>> Could not connect to %s <<<\n",  
                                                tm.tm_hour, tm.tm_min, tm.tm_sec, port);
                    consoleBufferSize += advance;
                }
            }
            TermSameLine(term);
            TermInputText(term, "Port", port, PORT_MAX_LENGTH, PlatformTerminalInputTextFlags_CharsNoBlank);
        }

        int bitsPerSecond = bytesRead * 8;

        TermPrintf(term, "BAUD: 115200");
        TermPrintf(term, "RX Rate: %d", bitsPerSecond);
        TermPrintf(term, "Console Size: %.1f / %.1f KB", (float) consoleBufferSize / 1024.0f, (float) (CONSOLE_BUFFER_SIZE) / 1024.0f);

        TermHeaderStop(term);
        TermBodyStart(term);

        TermPrintBuffer(term, consoleBuffer, consoleBufferSize);
        if (TermInputText(term, "", (char*) txBuffer, READ_BUFFER_SIZE, PlatformTerminalInputTextFlags_AutoSelectAll | PlatformTerminalInputTextFlags_EnterReturnsTrue))
        {
            s32 len = (s32) strlen((char*) txBuffer);

            int advance = stbsp_sprintf((char*) (consoleBuffer + consoleBufferSize), 
                                        ">>> %.*s\r\n", len, (char*) txBuffer);

            consoleBufferSize += advance;

            while (len >= 0)
            {
                txBuffer[len--] = 0;
            }
        }

        if (bytesRead)
        {
            time_t t     = time(NULL);
            struct tm tm = *localtime(&t);

            int advance = stbsp_sprintf((char*) (consoleBuffer + consoleBufferSize), 
                                        "[%d:%d:%d] %.*s\r\n",  
                                        tm.tm_hour, tm.tm_min, tm.tm_sec, 
                                        bytesRead, readBuffer);

            consoleBufferSize += advance;
        }

        TermBodyStop(term);
        TermFrameStop(term);
    }

    InterfaceStop(interface);
    TermStop(term);

    free(consoleBuffer);
    free(port);

	return 0;
}
