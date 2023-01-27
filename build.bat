@echo off
setlocal enabledelayedexpansion

:: Check if the compiler is already in the path
where /q cl.exe

:: If it is, build the program
IF NOT ERRORLEVEL 1 GOTO BUILD

:: Otherwise, try to find the compiler using vswhere
:: https://github.com/microsoft/vswhere
FOR /F "usebackq tokens=*" %%i IN (`.\tools\vswhere.exe -products * -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "VC\**\Build\vcvars64.bat"`) DO (
    :: Call vcvars64.bat to set the environment variables and build the program
    CALL "%%i"
    GOTO BUILD
)

GOTO NOTFOUND

:BUILD
:: Build the program using the MSVC compiler
:: /O2 = Optimize for speed, /Zi = Generate debug information, /W4 = Warning level 4, /Feplist.exe = Output program: plist.exe
cl.exe /O2 /Zi /W4 /Feplist.exe *.c
GOTO END

:NOTFOUND
echo "MSVC not found"

:END
:: Test if the script was called from the command line or from Windows Explorer
echo %cmdcmdline% | findstr /I /C:"/c" >NUL 2>NUL

:: If the script was called from the command line (no /c in starting command), exit immediately
IF ERRORLEVEL 1 GOTO :EOF

:: Otherwise, pause before exiting
pause
