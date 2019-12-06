PLATFORM_TERM_INIT(TermInit)
{
    LinuxTerminalState *state = (LinuxTerminalState*) malloc(sizeof(LinuxTerminalState));
    LinuxInterfaceState *interface = &state->interface;

    initscr();

    int fd = open(portName, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        fprintf(stderr, "error %d opening %s: %s\n", errno, portName, strerror(errno));
        return -1;
    }

    interface->fd = fd;

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