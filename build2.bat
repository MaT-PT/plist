@echo off
setlocal enabledelayedexpansion

for /f "usebackq tokens=*" %%i in (`vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "VC\**\Build\vcvars64.bat"`) do (
    call "%%i"
    GOTO FOUND
)

GOTO NOTFOUND

:FOUND
cl /Zi /EHsc /nologo /Feplist.exe *.c
GOTO END

:NOTFOUND
echo "MSVC not found"


:END
pause
