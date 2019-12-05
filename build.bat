@echo off

where cl 2> NUL 1>NUL
if %ERRORLEVEL% neq 0 call dev

set INCLUDE_DIRS=
set LIBRARIES=opengl32.lib 

set CPP_DEFS=/D_CRT_SECURE_NO_WARNINGS /DTERM_UI

set CPP_FLAGS_DEBUG=/Od /DDEBUG /Zi
set CPP_FLAGS_REL=/O2 /Oi

set CPP_FLAGS=/nologo /MD /diagnostics:column  %CPP_DEFS% /WL /GR- /EHa- /W4 /wd4996 /wd4201 /wd4706 %INCLUDE_DIRS% 
set LD_FLAGS=%LIBRARIES% 

mkdir build 2> NUL

pushd build\

rem copy ..\libs\SDL2\lib\x64\*.dll 1>NUL
rem ..\libs\imgui\*.cpp

cl %CPP_FLAGS% %CPP_FLAGS_REL% ..\src\main.cpp /FePiTerm /link %LD_FLAGS%
echo Done...

popd
