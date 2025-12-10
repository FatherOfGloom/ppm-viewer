if not exist lib mkdir lib

curl -fsSL -o SDL2-devel-2.32.10-mingw.zip https://www.libsdl.org/release/SDL2-devel-2.32.10-mingw.zip
tar -xf SDL2-devel-2.32.10-mingw.zip
move SDL2-2.32.10\x86_64-w64-mingw32 lib\SDL2
rmdir /s /q SDL2-2.32.10
del SDL2-devel-2.32.10-mingw.zip

@REM if not exist lib\SDL2\tmp mkdir lib\SDL2\tmp
@REM move lib\SDL2\include lib\SDL2\tmp\SDL2
@REM move lib\SDL2\tmp\SDL2 lib\SDL2\include