@echo off

set TARGET_NAME=ppm.exe
set SRC_FILENAMES=main.c 
set ROOT_FOLDER=C:\Users\khan\dev\ppmviewer\
set CFLAGS=-Wall -Wextra -pedantic -ggdb -fanalyzer -std=c11 -lws2_32

setlocal enabledelayedexpansion

set SRC_PATHS=

for %%i in (%SRC_FILENAMES%) do (set SRC_PATHS=!SRC_PATHS! %ROOT_FOLDER%src\%%i)

echo %SRC_PATHS%
echo %LINK_FLAGS%

pushd %ROOT_FOLDER%

if not exist bin mkdir bin

if not exist bin\SDL2.dll copy lib\SDL2\bin\SDL2.dll bin\SDL2.dll

@echo on
gcc -Ilib\SDL2\include -Llib\SDL2\lib %SRC_PATHS% -lmingw32 -lSDL2main -lSDL2 -o bin/%TARGET_NAME%  %CFLAGS%
@echo off

if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo build success.

popd
endlocal