#include "platform_linux.h"

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h> 
#include <stdio.h> 

#include <stdarg.h>

#define MAX_PATH (4096)

PLATFORM_READ_FILE_CONTENTS(PlatformReadFileContents)
{
    FileContents result = {};
    
    return result;
}

PLATFORM_FREE_FILE_CONTENTS(PlatformFreeFileContents)
{
    if (file->contents)
    {
        free(file->contents);
    }
}

PLATFORM_GET_DUMMY_TARGET(PlatformGetDummyTarget)
{
    // This targets the USB port on my PC, maybe in the future try to guess what the correct port is
    char *target = "/dev/usbtty0";
    
    return target;
}

PLATFORM_SLEEP_MS(PlatformSleepMS)
{
    struct timespec ts;
    int res;

    ts.tv_sec  = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

PLATFORM_GET_TIME(PlatformGetTime)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6);
    if (ms > 999) 
    {
        s++;
        ms = 0;
    }

    return (void*) (((u64) s * 1000) + ms);
}

PLATFORM_TIME_TO_MS(PlatformTimeToMS)
{
    u64 time = (u64) _time;

    double result = (double) time / 1000.0;

    return (double) result;
}

PLATFORM_DIR_ITERATOR(PlatformDirectoryIterator)
{
    PlatformFileIterator result = {};

    DIR *dir = opendir(path);
    struct dirent *entry = readdir(dir);

    if (dir && entry)
    {
        result._platform   = (void*) dir;
        result.currentFile = (PlatformFileIndex*) entry;

        result.isValid = true;
    }
    else
    {
        result.isValid = false;
    }

    return result;
}

PLATFORM_DIR_ITERATOR_NEXT(PlatformDirectoryIteratorNext)
{
    DIR *dir = (DIR*) iter->_platform;
    struct dirent *entry = readdir(dir);

    iter->currentFile = (PlatformFileIndex*) entry;

    if (iter->currentFile)
    {
        return iter->currentFile;
    }

    return 0;
}

PLATFORM_DIR_ITERATOR_CLOSE(PlatformDirectoryIteratorClose)
{
    if (iter->isValid)
    {
        closedir((DIR*) iter->_platform);
    }
}

PLATFORM_FILE_INDEX_GET_NAME(PlatformFileIndexGetName)
{
    struct dirent *entry = (struct dirent*) file;

    return entry->d_name;
}

PLATFORM_FILE_INDEX_GET_SIZE(PlatformFileIndexGetSize)
{
    return 0;
}

PLATFORM_FILE_INDEX_IS_DIR(PlatformFileIndexIsDir)
{
    struct dirent *entry = (struct dirent*) file;

    return (entry->d_type == DT_DIR);
}

PLATFORM_GET_EXE_DIRECTORY(PlatformGetEXEDirectory)
{
    char *result = (char*) calloc(MAX_PATH, 1);

    readlink("/proc/self/exe", result, MAX_PATH);

    size_t len = strlen(result);
    char *index = result + (len);

    while (*index != '/')
    {
        --index;
    }

    *(index) = '\0';

    return result;
}

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

    speed_t _baud = B115200;
    switch (baud)
    {
        case INTERFACE_BAUD_9600:
            _baud = B9600;
        break;

        case INTERFACE_BAUD_115200:
            _baud = B115200;
        break;
    }

    cfsetospeed(&tty, _baud);
    cfsetispeed(&tty, _baud);

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
        *errorCode = 1;

        return interface;
    }

    int error = InterfaceSetAttribs(interface, baud);
    error |= InterfaceSetBlocking(interface, false);

    if (error)
    {
        *errorCode = 1;

        return interface;
    }

    return interface;
}

PLATFROM_INTERFACE_REINIT(InterfaceReInit)
{
    LinuxInterfaceState *interface = (LinuxInterfaceState*) _interface;

    interface->fd = open(portName, O_RDWR | O_NOCTTY | O_SYNC);
    if (interface->fd < 0)
    {
        fprintf(stderr, "error %d opening %s: %s\n", errno, portName, strerror(errno));

        return 1;
    }

    int error = InterfaceSetAttribs(interface, baud);
    error |= InterfaceSetBlocking(interface, false);

    if (error)
    {
        return 1;
    }

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

PLATFORM_INTERFACE_DISCONENCT(InterfaceDisconnect)
{
    // TODO(Peacock): Do we have to restore any terminal settings?
    LinuxInterfaceState *interface = (LinuxInterfaceState*) _interface;

    if (interface->fd != -1)
    {
        close(interface->fd);
    }
    
    return 0;
}

#include "terminal_imgui.cpp"
