#if !defined(PI_Term_H)
#define PI_Term_H

#include <stdint.h>

#define GLOBAL static
#define INTERNAL static
#define PRESISTANT static

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
typedef void* TimeCount;

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

typedef u32 PlatformTerminalInputTextFlags;
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

typedef u32 PlatformTerminalResult;
enum PlatformTerminalResult_
{
    PlatformTerminalResult_None = 0,

    PlatformTerminalResult_Error = 1 << 0,
    PlatformTerminalResult_Fatal = 1 << 1,

    PlatformTerminalResult_ClearConsole = 1 << 2,
    PlatformTerminalResult_Quit         = 1 << 3,
    PlatformTerminalResult_HasResult    = 1 << 4,
};

enum PlatformInterfaceBaudRate
{
    INTERFACE_BAUD_9600,
    INTERFACE_BAUD_115200
};

typedef void* PlatformFileIndex;
struct PlatformFileIterator
{
    bool isValid;
    
    PlatformFileIndex *currentFile;
    
    void *_platform;
};

struct FileContents
{
    u8 *contents;
    u32 size;
};

// 
// General platform shit
//
#define PLATFORM_GET_TIME(name) TimeCount name()
#define PLATFORM_TIME_TO_MS(name) double name(TimeCount _time)
#define PLATFORM_GET_EXE_DIRECTORY(name) char* name()

#define PLATFORM_DIR_ITERATOR(name) PlatformFileIterator name(char *path)
#define PLATFORM_DIR_ITERATOR_CLOSE(name) void name(PlatformFileIterator *iter)
#define PLATFORM_DIR_ITERATOR_NEXT(name) PlatformFileIndex* name(PlatformFileIterator *iter)

#define PLATFORM_FILE_INDEX_GET_NAME(name) char* name(PlatformFileIndex *file)
#define PLATFORM_FILE_INDEX_GET_SIZE(name) u64 name(PlatformFileIndex *file)
#define PLATFORM_FILE_INDEX_IS_DIR(name) bool name(PlatformFileIndex *file)

#define PLATFORM_SLEEP_MS(name) void name(u32 millis)

#define PLATFORM_GET_DUMMY_TARGET(name) char* name()

#define PLATFORM_READ_FILE_CONTENTS(name) FileContents name(const char *path)
#define PLATFORM_FREE_FILE_CONTENTS(name) void name(FileContents *file)

//
// Interface (UART) 
//
#define PLATFORM_INTERFACE_INIT(name) Interface name(int *errorCode, char *portName, PlatformInterfaceBaudRate baud = INTERFACE_BAUD_115200)
#define PLATFORM_INTERFACE_STOP(name) int name(Interface _interface)
#define PLATFROM_INTERFACE_REINIT(name) int name(Interface _interface, char *portName, PlatformInterfaceBaudRate baud = INTERFACE_BAUD_115200)
#define PLATFORM_INTERFACE_DISCONENCT(name) int name(Interface _interface)

#define PLATFORM_INTERFACE_READ(name) int name(Interface _interface, u8 *buffer, u32 bufferSize)
#define PLATFORM_INTERFACE_WRITE(name) int name(Interface _interface, u8 *buffer, u32 bufferSize)
#define PLATFORM_INTERFACE_SET_ATTRIBS(name) int name(Interface _interface, PlatformInterfaceBaudRate baud)
#define PLATFORM_INTERFACE_SET_BLOCKING(name) int name(Interface _interface, bool shouldBlock)

// Interface helpers


//
// Terminal
//
#define PLATFORM_TERM_INIT(name) Term name(int *errorCode)
#define PLATFORM_TERM_STOP(name) PlatformTerminalResult name(Term _term)

#define PLATFORM_TERM_FRAME_START(name) PlatformTerminalResult name(Term _term)
#define PLATFORM_TERM_FRAME_STOP(name) PlatformTerminalResult name(Term _term)

#define PLATFORM_TERM_HEADER_START(name) PlatformTerminalResult name(Term _term)
#define PLATFORM_TERM_HEADER_STOP(name) PlatformTerminalResult name(Term _term)

#define PLATFORM_TERM_BODY_START(name) PlatformTerminalResult name(Term _term)
#define PLATFORM_TERM_BODY_STOP(name) PlatformTerminalResult name(Term _term)

#define PLATFORM_TERM_BOOTLOADER_START(name) bool name(Term _term)
#define PLATFORM_TERM_BOOTLOADER_STOP(name) PlatformTerminalResult name(Term _term)


#define PLATFORM_TERM_MESSAGE_BOX(name) PlatformTerminalResult name(Term _term, const char *title, const char *fmt, ...)

#define PLATFORM_TERM_SAME_LINE(name) PlatformTerminalResult name(Term _term)
#define PLATFORM_TERM_BUTTON(name) PlatformTerminalResult name(Term _term, char *label)

#define PLATFORM_TERM_PRINTF(name) PlatformTerminalResult name(Term _term, const char *fmt, ...)
#define PLATFORM_TERM_PRINT_BUFFER(name) PlatformTerminalResult name(Term _term, u8 *buffer, u32 bufferSize)

#define PLATFORM_TERM_INPUT_TEXT(name) PlatformTerminalResult name(Term _term, char *label, char *buffer, u32 bufferSize, PlatformTerminalInputTextFlags flags)

#define PLATFORM_TERM_FILE_SELECTOR(name) PlatformTerminalResult name(Term _term, char *rootPath, char *result)

#define PLATFORM_TERM_GET_BOOTLOADER_FILE_PATH(name) char* name(Term _term)
#define PLATFORM_TERM_SET_BOOTLOADER_FILE_PATH(name) PlatformTerminalResult name(Term _term, char *path)

#define PLATFORM_TERM_GET_BOOTLOADER_ROOT_PATH(name) char* name(Term _term)
#define PLATFORM_TERM_SET_BOOTLOADER_ROOT_PATH(name) PlatformTerminalResult name(Term _term, char *path)

#define PLATFORM_TERM_GET_BOOTLOADER_SELECTED_PATH(name) char* name(Term _term)


#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "imgui.h"

struct TerminalState
{
	SDL_Window *window;
	SDL_GLContext glContext;

    v4 clearColor;

    char *bootloaderInputRootPath;
    char *bootloaderInputFilePath;
    char *bootloaderSelectedPath;

	bool scrollLock;

    bool openSettings;
    bool openBootloader;
};

#if defined(_WIN32)
    #include "platform_win32.cpp"
#else
    #include "platform_linux.cpp"
#endif
#endif
