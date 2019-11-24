#include <stdio.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <ncurses.h>

#include "PiTerm.h"

// https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c

int
SetInterfaceAttribs(int fd, int speed, int parity)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcgetattr\n", errno);
        return -1;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

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
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcsetattr\n", errno);
        return -1;
    }

    return 0;
}

void
SetBlocking(int fd, int should_block)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
    	fprintf(stderr, "error %d setting term attributes", errno);
    }
}

int
main(int argc, char **argv)
{
	UNUSED(argc);

	if (argc <= 1)
	{
		printf("I need a portname (ie /dev/ttyS4)!\n");
		return 0;
	}

    initscr();

	char *port = argv[1];

	mvprintw(0, 0, "Trying to use port %s...\n", port);

	int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
        fprintf(stderr, "error %d opening %s: %s\n", errno, port, strerror(errno));
        return -1;
	}

	SetInterfaceAttribs(fd, B115200, 0);  // set speed to 115200 bps, 8n1 (no parity)
	SetBlocking(fd, 0);                 // set non-blocking

#define TERM_BUFFER_SIZE (1024 * 1024)

    char *termBuffer = (char*) malloc(TERM_BUFFER_SIZE);
    unsigned termBufferSize = 0;

    bool running = true;
    while (running)
    {
        move(0, 0);
        printw("PORT: %s\n", port);
        printw("BAUD: 115200\n");
        
        if (termBufferSize >= TERM_BUFFER_SIZE)
        {
            termBufferSize = 0;
            erase();
        }

        // We've read data from the UART
        int bytesRead = read(fd, termBuffer + termBufferSize, TERM_BUFFER_SIZE - termBufferSize);
        if (bytesRead)
        {
            printw("RX Rate: %d\n", bytesRead);
            printw("Buffer: %d / %d\n", termBufferSize, TERM_BUFFER_SIZE);
            
            termBufferSize += bytesRead;

            printw("~~~~~~~~~~~~~\n");
            printw("%.*s", termBufferSize, termBuffer);
        }

        refresh();
    }

    free(termBuffer);
	
	// int red;
	// while ((red = read(fd, outBuffer, 512)))
	// {
	// 	printf("%.*s", red, outBuffer);
 //        red = read(0, inBuffer, 512);

 //        if (red)
 //        {
 // //            printf("[DEBUG] %.*s\n", (int) red, inBuffer);
 // //            write(fd, inBuffer, red);
 // //        }
	// // }

    endwin();
	return 0;
}