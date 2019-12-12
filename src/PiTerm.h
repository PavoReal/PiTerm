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

struct v4
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };

        struct
        {
            float r;
            float g;
            float b;
            float a;
        };

        float _d[4];
    };
};

typedef u8 PlatformTerminalInputTextFlags;
enum PlatformTerminalInputTextFlags_
{
    PlatformTerminalInputTextFlags_None                = 0,
    PlatformTerminalInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    PlatformTerminalInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    PlatformTerminalInputTextFlags_CharsUppercase      = 1 << 2,   // Turn a..z into A..Z
    PlatformTerminalInputTextFlags_CharsNoBlank        = 1 << 3,   // Filter out spaces, tabs
    PlatformTerminalInputTextFlags_AutoSelectAll       = 1 << 4,   // Select entire text when first taking mouse focus
    PlatformTerminalInputTextFlags_EnterReturnsTrue    = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit() function.
    PlatformTerminalInputTextFlags_AllowTabInput       = 1 << 10,  // Pressing TAB input a '\t' character into the text field
    PlatformTerminalInputTextFlags_CtrlEnterForNewLine = 1 << 11,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).
    PlatformTerminalInputTextFlags_NoHorizontalScroll  = 1 << 12,  // Disable following the cursor horizontally
    PlatformTerminalInputTextFlags_AlwaysInsertMode    = 1 << 13,  // Insert mode
    PlatformTerminalInputTextFlags_ReadOnly            = 1 << 14,  // Read-only mode
    PlatformTerminalInputTextFlags_Password            = 1 << 15,  // Password mode, display all characters as '*'
    PlatformTerminalInputTextFlags_NoUndoRedo          = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().
    PlatformTerminalInputTextFlags_CharsScientific     = 1 << 17,  // Allow 0123456789.+-*/eE (Scientific notation input)
};

enum PlatformInterfaceBaudRate
{
    INTERFACE_BAUD_9600,
    INTERFACE_BAUD_115200
};

//
// Interface (UART) 
//
#define PLATFORM_INTERFACE_INIT(name) Interface name(int *errorCode, char *portName, PlatformInterfaceBaudRate baud = INTERFACE_BAUD_115200)
#define PLATFORM_INTERFACE_STOP(name) int name(Interface _interface)
#define PLATFROM_INTERFACE_REINIT(name) int name(Interface _interface, char *portName)
#define PLATFORM_INTERFACE_DISCONENCT(name) int name(Interface _interface)

#define PLATFORM_INTERFACE_READ(name) int name(Interface _interface, u8 *buffer, u32 bufferSize)
#define PLATFORM_INTERFACE_WRITE(name) int name(Interface _interface, u8 *buffer, u32 bufferSize)
#define PLATFORM_INTERFACE_SET_ATTRIBS(name) int name(Interface _interface, PlatformInterfaceBaudRate baud)
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

#define PLATFORM_TERM_SAME_LINE(name) int name(Term _term)
#define PLATFORM_TERM_BUTTON(name) bool name(Term _term, char *label)

#define PLATFORM_TERM_PRINTF(name) int name(Term _term, const char *fmt, ...)
#define PLATFORM_TERM_PRINT_BUFFER(name) int name(Term _term, u8 *buffer, u32 bufferSize)

#define PLATFORM_TERM_INPUT_TEXT(name) int name(Term _term, char *label, char *buffer, u32 bufferSize, PlatformTerminalInputTextFlags flags)

#if defined(TERM_GUI)
	#define SDL_MAIN_HANDLED
	#include <SDL.h>
	#include "imgui.h"

	struct TerminalState
	{
		SDL_Window *window;
		SDL_GLContext glContext;

        v4 clearColor;

		bool scrollLock;
        bool openSettings;
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
