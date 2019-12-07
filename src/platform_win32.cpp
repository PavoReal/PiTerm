#include "platform_win32.h"

#include <string.h>
#include <stdlib.h>

#include <stdarg.h>

PLATFORM_INTERFACE_READ(InterfaceRead)
{
    int bytesRead = 0;



    return bytesRead;
}

PLATFORM_INTERFACE_WRITE(InterfaceWrite)
{
    int bytesWritten = 0;

    return bytesWritten;
}

PLATFORM_INTERFACE_SET_ATTRIBS(InterfaceSetAttribs)
{
    return 0;
}

PLATFORM_INTERFACE_SET_BLOCKING(InterfaceSetBlocking)
{
    return 0;
}

PLATFORM_INTERFACE_INIT(InterfaceInit)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) malloc(sizeof(WIN32InterfaceState));

    HANDLE handle = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Could not open %s\n", portName);

        *errorCode = 1;
        return 0;
    }

    *errorCode = 0;
    return interface;
}

PLATFORM_INTERFACE_STOP(InterfaceStop)
{
    free(_interface);

    return 0;
}

#include "platform_imgui.cpp"

