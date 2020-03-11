@echo off

where cl 2> NUL 1> NUL
if %ERRORLEVEL% neq 0 call dev

set INCLUDE_DIRS=/I..\libs\win32\include
set LIBRARIES=opengl32.lib SDL2.lib 

set CPP_DEFS=/D_CRT_SECURE_NO_WARNINGS /DPI_TERM /DPI_TERM_INTERNAL

set CPP_FLAGS_DEBUG=/Od /DDEBUG /Zi
set CPP_FLAGS_REL=/O2 /Oi

set LD_FLAGS_DEBUG=/DEBUG 
set LD_FLAGS_RELEASE=/LTCG

set CPP_FLAGS=/nologo /diagnostics:column %CPP_DEFS% /MD /WL /GR- /EHa- /W4 /wd4996 /wd4201 /wd4706 /wd4068 %INCLUDE_DIRS% 
set LD_FLAGS=/nologo %LIBRARIES%

mkdir build 2> NUL

pushd build\

del *.obj 2> NUL

copy ..\libs\win32\release\SDL2.lib .\ > NUL
copy ..\libs\win32\release\SDL2.dll .\ > NUL

cl /c %CPP_FLAGS% %CPP_FLAGS_REL% ..\src\main.cpp 
if %ERRORLEVEL% neq 0 goto done

cl /c %CPP_FLAGS% %CPP_FLAGS_REL% ..\src\sha1\sha1.c
cl /c %CPP_FLAGS% %CPP_FLAGS_REL% ..\src\gl\gl3w.c
cl /c %CPP_FLAGS% %CPP_FLAGS_REL% ..\src\imgui\imgui*.cpp

link %LD_FLAGS% %LD_FLAGS_DEBUG% *.obj /OUT:PiTerm.exe

echo Done...

:done
popd
