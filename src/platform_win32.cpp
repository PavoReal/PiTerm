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
    WIN32InterfaceState *interface = (WIN32InterfaceState*) VirtualAlloc(NULL, sizeof(WIN32InterfaceState),
                                                                         MEM_COMMIT | MEM_RESERVE, 0);

    *_interface = interface;
    return 0;
}

PLATFORM_INTERFACE_STOP(InterfaceStop)
{
    VirtualFree(_interface, 0, MEM_RELEASE);

    return 0;
}

#include "platform_imgui.cpp"

