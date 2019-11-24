#include <stdio.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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

	int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
        fprintf(stderr, "error %d opening %s: %s\n", errno, port, strerror(errno));
        return -1;
	}

	SetInterfaceAttribs(fd, B115200, 0);  // set speed to 115200 bps, 8n1 (no parity)
	SetBlocking(fd, 0);                 // set non-blocking

    int width, height;
    getmaxyx(stdscr, height, width);

    
    WINDOW *headerWindow = newwin(10, width - 1, 2, 0);
    WINDOW *header = subwin(headerWindow, 9, width - 2, 2, 0);

    box(headerWindow, 0, 0);

    touchwin(headerWindow);
    wrefresh(headerWindow);

    WINDOW *termWindow   = newwin(height - 16, width - 1, 15, 0);
    WINDOW *term = subwin(termWindow, height - 30, width - 2, 16, 0);

    scrollok(term, true);

#define BUFFER_SIZE (1024)
    char buffer[BUFFER_SIZE];

    bool running = true;
    while (running)
    {
        int bytesRead = read(fd, buffer, BUFFER_SIZE);

        if (bytesRead > 0)
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);

            int bitsPerSecond = bytesRead * 8;

            mvwprintw(header, 1, 1, "BAUD: 115200");
            mvwprintw(header, 2, 1, "RX Rate: %d", bitsPerSecond);

            wrefresh(header);

            wprintw(term, "[%d:%d:%d] %.*s",  tm.tm_hour, tm.tm_min, tm.tm_sec, bytesRead, buffer);

            wrefresh(term);
        }

        bytesRead = read(0, buffer, BUFFER_SIZE);

        if (bytesRead)
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);

            wprintw(term, "[%d:%d:%d][TX] %.*s",  tm.tm_hour, tm.tm_min, tm.tm_sec, bytesRead, buffer);
        }
    }

    endwin();
	return 0;
}