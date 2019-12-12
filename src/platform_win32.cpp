#include "platform_win32.h"

#include <string.h>
#include <stdlib.h>

#include <stdarg.h>

PLATFORM_INTERFACE_READ(InterfaceRead)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) _interface;
    DWORD _bytesRead = {};

    if (interface->handle != INVALID_HANDLE_VALUE)
    {
        ReadFile(interface->handle, buffer, bufferSize, &_bytesRead, NULL);
    }

    return (int) _bytesRead;
}

PLATFORM_INTERFACE_WRITE(InterfaceWrite)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) _interface;
    DWORD _bytesWritten = {};

    if (interface != INVALID_HANDLE_VALUE)
    {
        BOOL success = WriteFile(interface->handle, buffer, bufferSize, &_bytesWritten, 0);

        if (!success)
        {
            fprintf(stderr, "Write failed!\n");
        }
    }

    return (int) _bytesWritten;
}

PLATFORM_INTERFACE_SET_ATTRIBS(InterfaceSetAttribs)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) _interface;

    DCB dcbSerialParams = { 0 }; // Initializing DCB structure
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // TODO(Peacock): Baud rate
    BOOL error = GetCommState(interface->handle, &dcbSerialParams);

    switch (baud)
    {
        case INTERFACE_BAUD_9600:
            dcbSerialParams.BaudRate = CBR_9600;
        break;

        case INTERFACE_BAUD_115200:
            dcbSerialParams.BaudRate = CBR_115200;
        break;
    }
    
    dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
    dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
    dcbSerialParams.Parity   = NOPARITY;  // Setting Parity = None

    error |= SetCommState(interface->handle, &dcbSerialParams);

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = MAXDWORD; // in milliseconds
    timeouts.ReadTotalTimeoutConstant    = 0; // in milliseconds
    timeouts.ReadTotalTimeoutMultiplier  = 0; // in milliseconds
    timeouts.WriteTotalTimeoutConstant   = 50; // in milliseconds
    timeouts.WriteTotalTimeoutMultiplier = 50; // in milliseconds

    error |= SetCommTimeouts(interface->handle, &timeouts);

    if (!error)
    {
        return 1;
    }

    return 0;
}

// PLATFORM_INTERFACE_SET_BLOCKING(InterfaceSetBlocking)
// {
//     return 0;
// }

PLATFORM_INTERFACE_INIT(InterfaceInit)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) malloc(sizeof(WIN32InterfaceState));

    interface->handle = CreateFile(portName, 
        GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (interface->handle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Could not open %s\n", portName);

        *errorCode = 1;
        return interface;
    }

    if (InterfaceSetAttribs(interface, baud))
    {
        fprintf(stderr, "Could not set interface attribs for %s\n", portName);
        *errorCode = 1;
    }
    else
    {
        *errorCode = 0;
    }

    return interface;
}

PLATFROM_INTERFACE_REINIT(InterfaceReInit)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) _interface;

    if (interface->handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(interface->handle);
    }

    interface->handle = CreateFile(portName, 
                                   GENERIC_READ | GENERIC_WRITE, 
                                   FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (interface->handle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Could not open %s\n", portName);

        return 1;
    }

    DCB dcbSerialParams = { 0 }; // Initializing DCB structure
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    BOOL error = GetCommState(interface->handle, &dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_115200;  // Setting BaudRate = 9600
    dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
    dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
    dcbSerialParams.Parity   = NOPARITY;  // Setting Parity = None

    error |= SetCommState(interface->handle, &dcbSerialParams);

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = MAXDWORD; // in milliseconds
    timeouts.ReadTotalTimeoutConstant    = 0; // in milliseconds
    timeouts.ReadTotalTimeoutMultiplier  = 0; // in milliseconds
    timeouts.WriteTotalTimeoutConstant   = 50; // in milliseconds
    timeouts.WriteTotalTimeoutMultiplier = 50; // in milliseconds

    error |= SetCommTimeouts(interface->handle, &timeouts);

    if (!error)
    {
        fprintf(stderr, "Could not set CommState for %s\n", portName);

        return 1;
    }

    return 0;
}

PLATFORM_INTERFACE_STOP(InterfaceStop)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) _interface;

    CloseHandle(interface->handle);
    free(interface);

    return 0;
}

PLATFORM_INTERFACE_DISCONENCT(InterfaceDisconnect)
{
    WIN32InterfaceState *interface = (WIN32InterfaceState*) _interface;

    CloseHandle(interface->handle);

    return 0;
}

#include "platform_imgui.cpp"
