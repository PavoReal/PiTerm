#include <stdio.h>
#include <time.h>

#pragma warning( push )
#pragma warning( disable : 4100 )
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define STB_SPRINTF_IMPLEMENTATION 
#include "stb_sprintf.h"

#pragma GCC diagnostic pop
#pragma warning( pop )

#include <stdarg.h>
#include "PiTerm.h"

inline void
AppendToBuffer(u8 *buffer, u32 *bufferSize, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int advance = stbsp_vsprintf((char*) (buffer + *bufferSize), fmt, args);
    *bufferSize += advance;

    va_end(args);
}

int
main(int argc, char **argv)
{
    SDL_SetMainReady();
        
#define PORT_MAX_LENGTH (KILOBYTES(1))
    char *port = (char*) malloc(PORT_MAX_LENGTH + 1);
	if (argc <= 1)
	{
        const char *dummyTarget = "/dev/ttyUSB0";
        strcpy(port, dummyTarget);
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

#define READ_BUFFER_SIZE (1024)
    u8 readBuffer[READ_BUFFER_SIZE];
    u8 txBuffer[READ_BUFFER_SIZE] = {};

#define CONSOLE_BUFFER_SIZE (MEGABYTES(10))
    u8 *consoleBuffer = (u8*) malloc(CONSOLE_BUFFER_SIZE);
    u32 consoleBufferSize = 0;

    Interface interface = InterfaceInit(&error, port);

    bool interfaceGood = (error == 0);
    if (!interfaceGood)
    {
        AppendToBuffer(consoleBuffer, &consoleBufferSize, ">>> Could not connect to %s <<<\n", port);
    }

    bool running = true;
    double lastFrameTime = 0;

    while (running)
    {
        TimeCount startTime = PlatformGetTime();

        if (TermFrameStart(term) == (PlatformTerminalResult_Fatal | PlatformTerminalResult_Quit))
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

                AppendToBuffer(consoleBuffer, &consoleBufferSize, ">>> Disconnected to %s <<<\n", port);
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

                if (interfaceGood)
                {
                    AppendToBuffer(consoleBuffer, &consoleBufferSize, ">>> Connected to %s <<<\n", port);
                }
                else
                {
                    AppendToBuffer(consoleBuffer, &consoleBufferSize, ">>> Could not connect to %s <<<\n", port);
                }
            }

            TermSameLine(term);
            TermInputText(term, "Port", port, PORT_MAX_LENGTH, PlatformTerminalInputTextFlags_CharsNoBlank);
        }

        int bitsPerSecond = bytesRead * 8;

        TermPrintf(term, "BAUD: 115200");
        TermPrintf(term, "RX Rate: %d", bitsPerSecond);
        TermPrintf(term, "Console Size: %.1f / %.1f KB", (float) consoleBufferSize / 1024.0f, (float) (CONSOLE_BUFFER_SIZE) / 1024.0f);
        TermPrintf(term, "Frame time: %lf ms", lastFrameTime);

        TermHeaderStop(term);

        if (TermBodyStart(term) == (PlatformTerminalResult_ClearConsole))
        {
            consoleBufferSize = 0;
        }

        TermPrintBuffer(term, consoleBuffer, consoleBufferSize);
        if (TermInputText(term, "", (char*) txBuffer, READ_BUFFER_SIZE, PlatformTerminalInputTextFlags_AutoSelectAll | PlatformTerminalInputTextFlags_EnterReturnsTrue))
        {
            s32 len = (s32) strlen((char*) txBuffer);

            AppendToBuffer(consoleBuffer, &consoleBufferSize, ">>> %.*s\n", len, (char*) txBuffer);

            u32 sizeToSend = len + 1;
            u8 *toSend = (u8*) malloc(sizeToSend);

            strcpy((char*) toSend, (char*) txBuffer);
            toSend[sizeToSend - 1] = '\0';

            InterfaceWrite(interface, toSend, sizeToSend);

            free(toSend);

            while (len >= 0)
            {
                txBuffer[len--] = 0;
            }
        }

        if (bytesRead)
        {
            time_t t     = time(NULL);
            struct tm tm = *localtime(&t);

            AppendToBuffer(consoleBuffer, &consoleBufferSize, "[%d:%d:%d] %.*s",  
                                        tm.tm_hour, tm.tm_min, tm.tm_sec, 
                                        bytesRead, readBuffer);

            if (consoleBuffer[consoleBufferSize - 1] != '\n')
            {
                AppendToBuffer(consoleBuffer, &consoleBufferSize, "\n");
            }
        }

        TermBodyStop(term);
        TermFrameStop(term);

        TimeCount endTime = PlatformGetTime();

        double startMS = PlatformTimeToMS(startTime);
        double endMS   = PlatformTimeToMS(endTime);

        lastFrameTime = endMS - startMS;
    }

    InterfaceStop(interface);
    TermStop(term);

    free(consoleBuffer);
    free(port);

	return 0;
}
