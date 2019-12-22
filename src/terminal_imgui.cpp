#include <stdlib.h>
#include <stdarg.h>

#include "gl3w.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "terminal_style_edit.cpp"

PLATFORM_TERM_INIT(TermInit)
{
    TerminalState *state = (TerminalState*) malloc(sizeof(TerminalState));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "Failed to init SDL2... %s\n", SDL_GetError());
        *errorCode = 1;
        
        return 0;
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
    SDL_GL_SetSwapInterval(1);

    if (gl3wInit() != 0)
    {
        fprintf(stderr, "Failed to inti gl3w...\n");
        *errorCode = 1;

        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "PiTerm.ini";

    ImGui::StyleColorsLight();
    
    ImGuiStyle &style = ImGui::GetStyle();
    
    style.FrameRounding   = 6;
    style.GrabRounding    = style.FrameRounding;
    *((float*) &style.WindowTitleAlign) = 0.5f;
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_FrameBg]  = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
    

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    state->window         = window;
    state->glContext      = gl_context;
    state->scrollLock     = true;
    state->openSettings   = false;
    state->openBootloader = true;
    state->clearColor     = { 0.45f, 0.55f, 0.6f, 1.00f };

    state->bootloaderInputRootPath = (char*) calloc(MAX_PATH, 1);
    state->bootloaderInputFilePath = (char*) calloc(MAX_PATH, 1);
    state->bootloaderSelectedPath  = (char*) calloc(MAX_PATH, 1);

    char *startPath = PlatformGetEXEDirectory();

    strcpy(state->bootloaderInputRootPath, startPath);

    free(startPath);

    *errorCode = 0;

    return state;
}

PLATFORM_TERM_STOP(TermStop)
{
    TerminalState *term = (TerminalState*) _term;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(term->glContext);
    SDL_DestroyWindow(term->window);
    SDL_Quit();

    free(term->bootloaderInputFilePath);
    free(term->bootloaderInputRootPath);
    free(term->bootloaderSelectedPath);
    free(term);

    return 0;
}

PLATFORM_TERM_FRAME_START(TermFrameStart)
{
    TerminalState *term = (TerminalState*) _term;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
        {
            return PlatformTerminalResult_Quit;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(term->window))
        {
            return PlatformTerminalResult_Quit;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(term->window);
    ImGui::NewFrame();

    return 0;
}

PLATFORM_TERM_FRAME_STOP(TermFrameStop)
{
    TerminalState *term = (TerminalState*) _term;

    ImGui::Render();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    glViewport(0, 0, (GLsizei) io.DisplaySize.x, (GLsizei) io.DisplaySize.y);
    glClearColor(term->clearColor.r, term->clearColor.g, term->clearColor.b, term->clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(term->window);

    return 0;
}

PLATFORM_TERM_HEADER_START(TermHeaderStart)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);

    ImGui::Begin("Info", NULL, ImGuiWindowFlags_MenuBar);

    ImGui::BeginMenuBar();
    ImGui::Checkbox("Settings", &term->openSettings);
    ImGui::Checkbox("Bootloader", &term->openBootloader);
    ImGui::EndMenuBar();

    return 0;
}

PLATFORM_TERM_HEADER_STOP(TermHeaderStop)
{
    TerminalState *term = (TerminalState*) _term;

    ImGui::End();

    if (term->openSettings)
    {
        ImGui::Begin("Settings");
        
        ShowStyleEditor(0, _term);
        
        ImGui::End();
    }

    return 0;
}

PLATFORM_TERM_BODY_START(TermBodyStart)
{
    TerminalState *term = (TerminalState*) _term;

    PlatformTerminalResult result = 0;

    ImGui::Begin("Console", NULL, ImGuiWindowFlags_MenuBar);

    ImGui::BeginMenuBar();

    ImGui::Checkbox("Scroll lock", &term->scrollLock);
    ImGui::Separator();
    if (ImGui::Button("Clear"))
    {
        result = PlatformTerminalResult_ClearConsole;
    }
    ImGui::Separator();
    if (ImGui::Button("Export"))
    {
        // TODO
    }

    ImGui::EndMenuBar();

    return result;
}

PLATFORM_TERM_BODY_STOP(TermBodyStop)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);

    if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
    {  
        ImGui::SetKeyboardFocusHere(0);
    }

    ImGui::End();

    return 0;
}

PLATFORM_TERM_BOOTLOADER_START(TermBootloaderStart) 
{
    TerminalState *term = (TerminalState*) _term;
    
    if (term->openBootloader)
    {
        ImGui::Begin("Bootloader");
    }
    
    return term->openBootloader;
}

PLATFORM_TERM_BOOTLOADER_STOP(TermBootloaderStop)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);
    
        ImGui::End();
    
    
    return 0;
}

PLATFORM_TERM_MESSAGE_BOX(TermMessageBox)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);
    
    va_list args;
    va_start(args, fmt);
    
    ImGui::OpenPopup(title);
    
    if (ImGui::BeginPopup(title))
    {
        ImGui::TextV(fmt, args);
        
        ImGui::EndPopup();
    }
    
    va_end(args);
    
    return 0;
}

PLATFORM_TERM_GET_BOOTLOADER_FILE_PATH(TermGetBootloaderFilePath)
{
    TerminalState *term = (TerminalState*) _term;
    
    return term->bootloaderInputFilePath;
}

PLATFORM_TERM_SET_BOOTLOADER_FILE_PATH(TermSetBootloaderFilePath)
{
    TerminalState *term = (TerminalState*) _term;
    
    strcpy(term->bootloaderInputFilePath, path);
    
    
    return 0;
}

PLATFORM_TERM_GET_BOOTLOADER_ROOT_PATH(TermGetBootloaderFileRootPath)
{
    TerminalState *term = (TerminalState*) _term;
    
    return term->bootloaderInputRootPath;
}

PLATFORM_TERM_SET_BOOTLOADER_FILE_PATH(TermSetBootloaderFileRootPath)
{
    TerminalState *term = (TerminalState*) _term;
    
    strcpy(term->bootloaderInputRootPath, path);
    
    return 0;
}

PLATFORM_TERM_GET_BOOTLOADER_SELECTED_PATH(TermGetBootloaderSelectedPath)
{
    TerminalState *term = (TerminalState*) _term;
    
    return term->bootloaderSelectedPath;
}

PLATFORM_TERM_PRINTF(TermPrintf)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);
    
    va_list args;
    va_start(args, fmt);

    ImGui::TextV(fmt, args);

    va_end(args);
    return 0;
}

PLATFORM_TERM_PRINT_BUFFER(TermPrintBuffer)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);

    ImGui::BeginChild((char*) buffer, ImVec2(0, 400), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::TextUnformatted((char*) buffer, (char*) (buffer + bufferSize));

    if (term->scrollLock)
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    return 0;
}

PLATFORM_TERM_SAME_LINE(TermSameLine)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);

    ImGui::SameLine();

    return 0;
}

PLATFORM_TERM_BUTTON(TermButton)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);

    bool pressed = false;

    pressed = ImGui::Button(label);

    return pressed;
}

PLATFORM_TERM_INPUT_TEXT(TermInputText)
{
    TerminalState *term = (TerminalState*) _term;
    UNUSED(term);

    bool result = ImGui::InputText(label, buffer, bufferSize, flags);
    return result;
}


PLATFORM_TERM_FILE_SELECTOR(TermFileSelector)
{
    TerminalState *term = (TerminalState*) _term;
    
    PlatformTerminalResult res = 0;
    
    ImGui::BeginChild(result, ImVec2(0,0), true);
    
    ImGui::InputText("", term->bootloaderInputRootPath, MAX_PATH);
    
    PlatformFileIterator iter = PlatformDirectoryIterator(rootPath);
    
    if (iter.isValid)
    {
        PlatformFileIndex *index = iter.currentFile;
        
        char dirBuffer[MAX_PATH];
         while(index)
        {
            char *name   = PlatformFileIndexGetName(index);
            u64 fileSize = PlatformFileIndexGetSize(index);
            bool isDir   = PlatformFileIndexIsDir(index);
            
            const char *dirMarker = "> ";
            
            if (isDir)
            {
                strcpy(dirBuffer, dirMarker);
                strcpy(dirBuffer + strlen(dirMarker), name);
            }
            else
            {
                strcpy(dirBuffer, name);
            }
            
            if (ImGui::Selectable(dirBuffer))
            {
                if (isDir)
                {
                    size_t len = strlen(term->bootloaderInputRootPath);
                    
                    if (strcmp(name, ".") == 0)
                    {
                        continue;
                    }
                    else if (strcmp(name, "..") == 0)
                    {
                        char *i = term->bootloaderInputRootPath + len;
                        
                        while (*i != *PLATFORM_FILE_SEPERATOR)
                        {
                            --i;
                        }
                        
                        *i = '\0';
                    }
                    else
                    {
                        strcpy(term->bootloaderInputRootPath + len, PLATFORM_FILE_SEPERATOR);
                        strcpy(term->bootloaderInputRootPath + strlen(term->bootloaderInputRootPath), name);
                    }
                }
                else
                {
                    strcpy(term->bootloaderSelectedPath, term->bootloaderInputRootPath);
                    strcpy(term->bootloaderSelectedPath + strlen(term->bootloaderSelectedPath), PLATFORM_FILE_SEPERATOR);
                    strcpy(term->bootloaderSelectedPath + strlen(term->bootloaderSelectedPath), name);
                res = PlatformTerminalResult_HasResult;
                }
            }
            
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "%lu bytes", fileSize);
            
            index = PlatformDirectoryIteratorNext(&iter);
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.9f, 0.05f, 0.05f, 1.0f), "Invalid path");
    }
    
    PlatformDirectoryIteratorClose(&iter);
    
    ImGui::EndChild();
    
    return res;
}
