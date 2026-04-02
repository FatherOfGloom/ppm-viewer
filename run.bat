@echo off

set TARGET_NAME=ppm.exe
set ROOT_FOLDER=%~dp0
set CLI_ARGS=%*

pushd %ROOT_FOLDER%
call ./build.bat

if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
echo:

pushd bin
%TARGET_NAME% %CLI_ARGS%
popd
popd