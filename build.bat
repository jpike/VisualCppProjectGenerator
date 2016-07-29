@ECHO off

REM PUT THE COMPILER IN THE PATH.
CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

REM MOVE INTO THE BUILD DIRECTORY.
IF NOT EXIST "build" MKDIR "build"
PUSHD "build"

    REM BUILD THE PROGRAM.
    REM This build.bat file currently only builds a basic debug build, so you'll need to tweak it if you want something else.
    REM See https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx for compiler options.
    REM /Zi - debug info
    REM /EHa - The exception-handling model that catches both asynchronous (structured) and synchronous (C++) exceptions.
    REM /WX - All warnings as errors
    REM /W4 - Warning level 4
    REM /MTd - Static linking with Visual C++ lib.
    REM user32.lib - Basic Windows functions.
    cl.exe /Zi /EHa /WX /W4 /MTd "..\GenerateProject.cpp" user32.lib

POPD

@ECHO ON
