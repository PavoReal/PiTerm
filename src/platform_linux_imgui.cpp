#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdarg.h>

PLATFORM_TERM_INIT(TermInit)
{
    return 0;
}

PLATFORM_TERM_STOP(TermStop)
{
    free(term);


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
    va_list args;
    va_start(args, fmt);


    va_end(args);
    return 0;
}

PLATFORM_TERM_PRINTPOS(TermPrintPos)
{
    va_list args;
    va_start(args, fmt);


    va_end(args);
    return 0;
}
