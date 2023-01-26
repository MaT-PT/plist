#pragma once

#include <Windows.h>
//

typedef struct SizeWithUnit {
    SIZE_T sSize;
    CHAR szUnit[4];
} SIZE_WITH_UNIT, *LPSIZE_WITH_UNIT;

typedef struct TimeSpan {
    DWORD wHours;
    WORD wMinutes;
    WORD wSeconds;
    WORD wMilliseconds;
} TIME_SPAN, *LPTIME_SPAN;

LPSTR RemoveExtension(IN CONST LPCSTR szFilename);

VOID GetSizeWithUnit(IN CONST SIZE_T sSize, OUT CONST LPSIZE_WITH_UNIT swuSize);

VOID TimeDeltaNsToTimeSpan(IN CONST ULONGLONG ullTimeNs, OUT CONST LPTIME_SPAN lpTimeSpan);

VOID PrintError(IN CONST LPCSTR lpFuncName);

BOOL AddSeDebugPrivileges(VOID);
