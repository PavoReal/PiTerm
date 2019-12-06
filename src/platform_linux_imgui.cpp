#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#include "gl3w.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

PLATFORM_TERM_INIT(TermInit)
{
    LinuxTerminalState *state = (LinuxTerminalState*) malloc(sizeof(LinuxTerminalState));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "Failed to init SDL2... %s\n", SDL_GetError());
        return -1;
    }

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("PiTerm", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    if (gl3wInit() != 0)
    {
        fprintf(stderr, "Failed to inti gl3w...\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    state->window = window;
    state->glContext = gl_context;
    state->io = io;

    *_term = state;

    return 0;
}

PLATFORM_TERM_STOP(TermStop)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(term->glContext);
    SDL_DestroyWindow(term->window);
    SDL_Quit();

    free(term);

    return 0;
}

PLATFORM_TERM_FRAME_START(TermFrameStart)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
        {
            return 1;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(term->window))
        {
            return 1;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(term->window);
    ImGui::NewFrame();

    return 0;
}

PLATFORM_TERM_FRAME_STOP(TermFrameStop)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;

    ImGui::Render();

    glViewport(0, 0, term->io.DisplaySize.x, term->io.DisplaySize.y);
    glClearColor(0.45f, 0.55f, 0.6f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(term->window);

    return 0;
}

PLATFORM_TERM_HEADER_START(TermHeaderStart)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;
    UNUSED(term);

    ImGui::Begin("Info");

    ImGui::Text("This is the header");
    
    return 0;
}

PLATFORM_TERM_HEADER_STOP(TermHeaderStop)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;
    UNUSED(term);

    ImGui::End();

    return 0;
}

PLATFORM_TERM_BODY_START(TermBodyStart)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;
    UNUSED(term);

    ImGui::Begin("Body");

    ImGui::Text("This is the body");

    return 0;
}

PLATFORM_TERM_BODY_STOP(TermBodyStop)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;
    UNUSED(term);
    
    ImGui::End();

    return 0;
}

PLATFORM_TERM_PRINTF(TermPrintf)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;
    UNUSED(term);
    
    va_list args;
    va_start(args, fmt);


    va_end(args);
    return 0;
}

PLATFORM_TERM_PRINTPOS(TermPrintPos)
{
    LinuxTerminalState *term = (LinuxTerminalState*) _term;
    UNUSED(term);
    
    va_list args;
    va_start(args, fmt);


    va_end(args);
    return 0;
}
