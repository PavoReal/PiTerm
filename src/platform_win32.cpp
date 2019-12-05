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

PLATFORM_TERM_INIT(TermInit)
{
    return 0;
}

PLATFORM_TERM_STOP(TermStop)
{
    return 0;
}

PLATFORM_TERM_HEADER_START(TermHeaderStart)
{
    return 0;
}

PLATFORM_TERM_HEADER_STOP(TermHeaderStop)
{
    return 0;
}

PLATFORM_TERM_BODY_START(TermBodyStart)
{
    return 0;
}

PLATFORM_TERM_BODY_STOP(TermBodyStop)
{
    return 0;
}

PLATFORM_TERM_PRINTF(TermPrintf)
{
    return 0;
}

PLATFORM_TERM_PRINTPOS(TermPrintPos)
{
    return 0;
}
