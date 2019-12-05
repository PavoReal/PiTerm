#include "platform_linux.h"

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdarg.h>

PLATFORM_INTERFACE_READ(InterfaceRead)
{
    int bytesRead = 0;

    LinuxTerminalState *state = (LinuxTerminalState*) term;

    bytesRead = read(state->fd, buffer, bufferSize);

    return bytesRead;
}

PLATFORM_INTERFACE_WRITE(InterfaceWrite)
{
    int bytesWritten = 0;

    LinuxTerminalState *state = (LinuxTerminalState*) term;

    bytesWritten = write(state->fd, buffer, bufferSize);

    return bytesWritten;
}

PLATFORM_INTERFACE_SET_ATTRIBS(InterfaceSetAttribs)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    int fd = state->fd;

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcgetattr\n", errno);
        return -1;
    }

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,
                                     // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    // tty.c_cflag |= 0; // No parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcsetattr\n", errno);
        return -1;
    }

    return 0;
}

PLATFORM_INTERFACE_SET_BLOCKING(InterfaceSetBlocking)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    int fd = state->fd;

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tggetattr", errno);
        return -1;
    }

    tty.c_cc[VMIN]  = shouldBlock ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
    	fprintf(stderr, "error %d setting term attributes", errno);
        return -1;
    }

    return 0;
}

PLATFORM_TERM_INIT(TermInit)
{
    LinuxTerminalState *state = (LinuxTerminalState*) malloc(sizeof(LinuxTerminalState));

    initscr();

    int fd = open(portName, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        fprintf(stderr, "error %d opening %s: %s\n", errno, portName, strerror(errno));
        return -1;
    }

    state->fd = fd;

    InterfaceSetAttribs(state, B115200);
    InterfaceSetBlocking(state, false);

    int width, height;
    getmaxyx(stdscr, height, width);

    WINDOW *headerWindow = newwin(10, width - 1, 2, 0);
    WINDOW *header = subwin(headerWindow, 9, width - 2, 2, 0);

    box(headerWindow, 0, 0);

    touchwin(headerWindow);
    wrefresh(headerWindow);

    WINDOW *termWindow = newwin(height - 16, width - 1, 15, 0);
    WINDOW *term = subwin(termWindow, height - 30, width - 2, 16, 0);

    scrollok(term, true);

    state->headerWindow = headerWindow;
    state->header       = header;
    state->termWindow = termWindow;
    state->term       = term;

    *platformTerm = (TERM) state;

    return 0;
}

PLATFORM_TERM_STOP(TermStop)
{
    free(term);

    endwin();

    return 0;
}

PLATFORM_TERM_HEADER_START(TermHeaderStart)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    state->activeWindow = state->header; 

    return 0;
}

PLATFORM_TERM_HEADER_STOP(TermHeaderStop)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    wrefresh(state->header);

    return 0;
}

PLATFORM_TERM_BODY_START(TermBodyStart)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    state->activeWindow = state->term; 

    return 0;
}

PLATFORM_TERM_BODY_STOP(TermBodyStop)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    wrefresh(state->term);

    return 0;
}

PLATFORM_TERM_PRINTF(TermPrintf)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    ASSERT(state->activeWindow);

    va_list args;

    va_start(args, fmt);

    vwprintw(state->activeWindow, fmt, args);

    va_end(args);

    return 0;
}

PLATFORM_TERM_PRINTPOS(TermPrintPos)
{
    LinuxTerminalState *state = (LinuxTerminalState*) term;
    ASSERT(state->activeWindow);

    va_list args;

    va_start(args, fmt);

    move(row, col);
    vwprintw(state->activeWindow, fmt, args);

    va_end(args);

    return 0;
}
