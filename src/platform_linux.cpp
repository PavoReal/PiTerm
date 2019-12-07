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

    LinuxInterfaceState *interface = (LinuxInterfaceState*) _interface;

    bytesRead = read(interface->fd, buffer, bufferSize);

    return bytesRead;
}

PLATFORM_INTERFACE_WRITE(InterfaceWrite)
{
    int bytesWritten = 0;

    LinuxInterfaceState *interface = (LinuxInterfaceState*) _interface;

    bytesWritten = write(interface->fd, buffer, bufferSize);

    return bytesWritten;
}

// https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
PLATFORM_INTERFACE_SET_ATTRIBS(InterfaceSetAttribs)
{
    LinuxInterfaceState *interface = (LinuxInterfaceState*) _interface;
 
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(interface->fd, &tty) != 0)
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

    if (tcsetattr(interface->fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcsetattr\n", errno);
        return -1;
    }

    return 0;
}

PLATFORM_INTERFACE_SET_BLOCKING(InterfaceSetBlocking)
{
    LinuxInterfaceState *interface = (LinuxInterfaceState*) _interface;
 
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(interface->fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tggetattr", errno);
        return -1;
    }

    tty.c_cc[VMIN]  = shouldBlock ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr(interface->fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "error %d setting term attributes", errno);
        return -1;
    }

    return 0;
}

PLATFORM_INTERFACE_INIT(InterfaceInit)
{
    LinuxInterfaceState *interface = (LinuxInterfaceState*) malloc(sizeof(LinuxInterfaceState));

    interface->fd = open(portName, O_RDWR | O_NOCTTY | O_SYNC);
    if (interface->fd < 0)
    {
        fprintf(stderr, "error %d opening %s: %s\n", errno, portName, strerror(errno));
        return -1;
    }

    InterfaceSetAttribs(interface, B115200);
    InterfaceSetBlocking(interface, false);

    *_interface = interface;

    return 0;
}

PLATFORM_INTERFACE_STOP(InterfaceStop)
{
    // TODO(Peacock): Do we have to restore any terminal settings?
    LinuxInterfaceState *interface = (LinuxInterfaceState*) _interface;

    close(interface->fd);
    free(interface);
    
    return 0;
}

#if defined(TERM_GUI)
    #include "platform_linux_imgui.cpp"
#else
    #include "platform_linux_ncurses.cpp"
#endif