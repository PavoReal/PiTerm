#if !defined(PI_Term_H)
#define PI_Term_H

#include <stdint.h>

#define UNUSED(a) (void) a

#if defined(DEBUG)
	#define ASSERT(a) if ((a)) { *(volatile int *)0 = 0; }
#else
	#define ASSERT(a)
#endif

#define KILOBYTES(a) (1024 * (a))
#define MEGABYTES(a) (1024 * KILOBYTES(a))

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef void* Term;
typedef void* Interface;

//
// Interface (UART) 
//
#define PLATFORM_INTERFACE_INIT(name) Interface name(int *errorCode, char *portName)
#define PLATFORM_INTERFACE_STOP(name) int name(Interface _interface)

#define PLATFORM_INTERFACE_READ(name) int name(Interface _interface, u8 *buffer, u32 bufferSize)
#define PLATFORM_INTERFACE_WRITE(name) int name(Interface _interface, u8 *buffer, u32 bufferSize)
#define PLATFORM_INTERFACE_SET_ATTRIBS(name) int name(Interface _interface, int baud)
#define PLATFORM_INTERFACE_SET_BLOCKING(name) int name(Interface _interface, bool shouldBlock)

//
// Terminal (UI)
//
#define PLATFORM_TERM_INIT(name) Term name(int *errorCode)
#define PLATFORM_TERM_STOP(name) int name(Term _term)

#define PLATFORM_TERM_FRAME_START(name) int name(Term _term)
#define PLATFORM_TERM_FRAME_STOP(name) int name(Term _term)

#define PLATFORM_TERM_HEADER_START(name) int name(Term _term)
#define PLATFORM_TERM_HEADER_STOP(name) int name(Term _term)
#define PLATFORM_TERM_BODY_START(name) int name(Term _term)
#define PLATFORM_TERM_BODY_STOP(name) int name(Term _term)

#define PLATFORM_TERM_PRINTF(name) int name(Term _term, const char *fmt, ...)
#define PLATFORM_TERM_PRINT_BUFFER(name) int name(Term _term, u8 *buffer, u32 bufferSize)

#if defined(TERM_GUI)
	#define SDL_MAIN_HANDLED
	#include <SDL.h>
	#include "imgui.h"

	struct TerminalState
	{
		SDL_Window *window;
		SDL_GLContext glContext;

		bool scrollLock;
	};
#endif

#if defined(_WIN32)
    #include "platform_win32.cpp"

    #if defined(TERM_GUI)
        #pragma message("Using IMGUI ui...")
    #else
        #error WIN32 build requires TERM_GUI...
    #endif
#else
    #include "platform_linux.cpp"

    #if defined(TERM_GUI)
        #pragma message("Using IMGUI ui...")
    #else
        #pragma message("Using ncurses ui...")
    #endif
#endif
#endif
