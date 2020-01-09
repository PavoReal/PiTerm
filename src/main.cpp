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
#include "Bootloader.h"
#include "libsha1.h"

#define AppendToConsoleBuffer(...) AppendToBuffer(consoleBuffer, &consoleBufferSize, CONSOLE_BUFFER_SIZE, __VA_ARGS__)
inline void
AppendToBuffer(u8 *buffer, u32 *bufferSize, u32 bufferMaxSize, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    char buf[4096];
    
    int advance = stbsp_vsprintf(buf, fmt, args);
    
    u32 newSize = *bufferSize + advance;
    
    if (newSize > bufferMaxSize)
    {
        *bufferSize = 0;
    }
    
    strcpy((char*) buffer + (*bufferSize), buf);
    
    *bufferSize += advance;

    va_end(args);
}

inline int
InterfaceWriteU32(Interface interface, u32 data) 
{
    return InterfaceWrite(interface, (u8*) &data, 4);
}

#define InterfaceWriteCommand(i,d) InterfaceWriteU8(i, d)
inline int
InterfaceWriteU8(Interface interface, u8 data) 
{
    return InterfaceWrite(interface, (u8*) &data, 1);
}

inline void
InterfaceEcho(Interface interface, const char *str)
{
    InterfaceWriteCommand(interface, BOOTLOADER_COMMAND_ECHO);
    InterfaceWrite(interface, (u8*) str, (u32) strlen(str) + 1);
}

inline bool
InterfaceWaitForAWK(Interface interface)
{
    bool error = false;
    
    BootloaderCommand rx = BOOTLOADER_COMMAND_UNKNOWN;
    
    u32 read;
    while (!(read = InterfaceRead(interface, &rx, sizeof(BootloaderCommand))))
    {
        // Do nothing
        PlatformSleepMS(1);
    }
    
    if (rx != BOOTLOADER_COMMAND_AWK)
    {
        error = true;
    }
    
    return error;
}

int
main(int argc, char **argv)
{
    SDL_SetMainReady();
        
#define PORT_MAX_LENGTH (KILOBYTES(1))
    char *port = (char*) malloc(PORT_MAX_LENGTH + 1);
	if (argc <= 1)
	{
        const char *dummyTarget = PlatformGetDummyTarget();
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
        AppendToConsoleBuffer(">>> Could not connect to %s <<<\n", port);
    }
    
    char *exeDir = PlatformGetEXEDirectory();
    TermSetBootloaderFileRootPath(term, exeDir);
    
    free(exeDir);
    
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
        
        int bytesRead = 0;
        {
        TermHeaderStart(term);

        if (interfaceGood)
        {
            bytesRead = InterfaceRead(interface, readBuffer, READ_BUFFER_SIZE);

            TermPrintf(term, "STATUS: Connected -- %s", port);
            TermSameLine(term);
            if (TermButton(term, "Disconnect"))
            {
                InterfaceDisconnect(interface);
                interfaceGood = false;

                AppendToConsoleBuffer(">>> Disconnected to %s <<<\n", port);
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
                        AppendToConsoleBuffer(">>> Connected to %s <<<\n", port);
                }
                else
                {
                        AppendToConsoleBuffer(">>> Could not connect to %s <<<\n", port);
                }
            }

            TermSameLine(term);
            TermInputText(term, "Port", port, PORT_MAX_LENGTH, PlatformTerminalInputTextFlags_CharsNoBlank);
        }

        int bitsPerSecond = bytesRead * 8;

            TermPrintf(term, "BAUD: 115200");
#if defined(DEBUG)
            TermPrintf(term, "RX Rate: %d", bitsPerSecond);
            #endif
            TermPrintf(term, "Console Size: %.1f / %.1f KB", (float) consoleBufferSize / 1024.0f, (float) (CONSOLE_BUFFER_SIZE) / 1024.0f);
            
#if defined(DEBUG) && 0
            TermPrintf(term, "Frame time: %lf ms", lastFrameTime);
            #endif

            TermHeaderStop(term);
        }
        
        {
            if (TermBootloaderStart(term))
            {
                char *targetFilePath = TermGetBootloaderFilePath(term);
                
                if (TermButton(term, "Upload"))
                {
                    FileContents file = PlatformReadFileContents(targetFilePath);
                    
                    if (file.size)
                    {
                        AppendToConsoleBuffer(">>> Loaded file %s with size %u bytes <<<\n", targetFilePath, file.size);
                        
                        u8 checksum[SHA1_DIGEST_SIZE];
                        sha1(checksum, file.contents, file.size);
                            
                        char tmpBuffer[4096];
                        stbsp_sprintf(tmpBuffer, "0x");
                        
                        for (int i = 0; i < SHA1_DIGEST_SIZE; ++i)
                        {
                            stbsp_sprintf(tmpBuffer + strlen(tmpBuffer), "%x%x", checksum[i] / 16, checksum[i] % 16);
                        }
                        
                        AppendToConsoleBuffer(">>> File checksum %s <<<\n", tmpBuffer);
                        
                        InterfaceWriteCommand(interface, BOOTLOADER_COMMAND_UPLOAD);
                        if (InterfaceWaitForAWK(interface))
                        {
                            AppendToConsoleBuffer(">>> Error sending BOOTLOADER_COMMAND_UPLOAD... <<<\n");
                        }
#if defined(DEBUG)
                        else
                        {
                            AppendToConsoleBuffer(">> Good awk -- upload command <<<\n");
                        }
                        #endif
                        
                        InterfaceWriteU32(interface, file.size);
                        if (InterfaceWaitForAWK(interface))
                        {
                            AppendToConsoleBuffer(">>> Error sending file size... <<<\n");
                        }
#if defined(DEBUG)
                        else
                        {
                            AppendToConsoleBuffer(">> Good awk -- file size <<<\n");
                        }
#endif
                        
                        
                        InterfaceWrite(interface, checksum, SHA1_DIGEST_SIZE);
                        if (InterfaceWaitForAWK(interface))
                        {
                            AppendToConsoleBuffer(">>> Error sending file checksum... <<<\n");
                        }
#if defined(DEBUG)
                        else
                        {
                            AppendToConsoleBuffer(">> Good awk -- checksum  <<<\n");
                        }
#endif
                        
// InterfaceWrite(interface, file.contents, file.size);
                        
                        PlatformFreeFileContents(&file);
                    }
                    else
                    {
                        AppendToConsoleBuffer(">>> Bootloader could not open file %s... <<<\n", targetFilePath);
                    }
                }
                
                TermSameLine(term);
                TermPrintf(term, "%s", targetFilePath);
                
                char *rootPath     = TermGetBootloaderFileRootPath(term);
                char *selectedPath = TermGetBootloaderSelectedPath(term);
                
                if (TermFileSelector(term, rootPath, selectedPath) == PlatformTerminalResult_HasResult)
                {
                    TermSetBootloaderFilePath(term, selectedPath);
                }
                
                TermBootloaderStop(term);
            }
        }
        
        {
        if (TermBodyStart(term) == (PlatformTerminalResult_ClearConsole))
        {
            consoleBufferSize = 0;
            }

        TermPrintBuffer(term, consoleBuffer, consoleBufferSize);
        if (TermInputText(term, "", (char*) txBuffer, READ_BUFFER_SIZE, PlatformTerminalInputTextFlags_AutoSelectAll | PlatformTerminalInputTextFlags_EnterReturnsTrue))
        {
            s32 len = (s32) strlen((char*) txBuffer);

                AppendToConsoleBuffer(">>> %.*s\n", len, (char*) txBuffer);

            u32 sizeToSend = len + 2;
            u8 *toSend = (u8*) malloc(sizeToSend);

            strcpy((char*) toSend, (char*) txBuffer);
                toSend[sizeToSend - 2] = '\n';
                toSend[sizeToSend - 1] = '\0';
                
                InterfaceEcho(interface, (char*) toSend);

            free(toSend);

            while (len >= 0)
            {
                txBuffer[len--] = 0;
            }
        }

        if (bytesRead)
            {
                AppendToConsoleBuffer("%.*s", bytesRead, readBuffer);
        }

            TermBodyStop(term);
        }
        TermFrameStop(term);

        TimeCount endTime = PlatformGetTime();

        double startMS = PlatformTimeToMS(startTime);
        double endMS   = PlatformTimeToMS(endTime);

        lastFrameTime = endMS - startMS;
        
        if (lastFrameTime <= 16)
        {
            double diff = 15 - lastFrameTime;
            
            PlatformSleepMS((u32) diff);
        }
    }

    InterfaceStop(interface);
    TermStop(term);

    free(consoleBuffer);
    free(port);

	return 0;
}
