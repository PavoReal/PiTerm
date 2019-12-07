@echo off

where cl 2> NUL 1>NUL
if %ERRORLEVEL% neq 0 call dev

set INCLUDE_DIRS=/I..\libs\win32\include
set LIBRARIES=opengl32.lib SDL2main.lib SDL2.lib 

set CPP_DEFS=/D_CRT_SECURE_NO_WARNINGS /DTERM_GUI

set CPP_FLAGS_DEBUG=/Od /DDEBUG /Zi
set CPP_FLAGS_REL=/O2 /Oi

set CPP_FLAGS=/nologo /MD /diagnostics:column  %CPP_DEFS% /WL /GR- /EHa- /W4 /wd4996 /wd4201 /wd4706 %INCLUDE_DIRS% 
set LD_FLAGS=/DEBUG %LIBRARIES%

mkdir build 2> NUL

pushd build\

copy ..\libs\win32\debug\*.lib .\

cl /c %CPP_FLAGS% %CPP_FLAGS_DEBUG% ..\src\main.cpp 
if %ERRORLEVEL% neq 0 goto done

cl /c %CPP_FLAGS% %CPP_FLAGS_DEBUG% ..\src\gl3w.c
cl /c %CPP_FLAGS% %CPP_FLAGS_DEBUG% ..\src\imgui*.cpp

link %LD_FLAGS%  *.obj /OUT:PiTerm.exe

echo Done...

:done
popd
