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

LPSTR RemoveExtension(CONST LPCSTR szFilename);

VOID GetSizeWithUnit(CONST SIZE_T sSize, CONST LPSIZE_WITH_UNIT swuSize);

VOID TimeDeltaNsToTimeSpan(CONST ULONGLONG ullTimeNs, CONST LPTIME_SPAN lpTimeSpan);

VOID PrintError(CONST LPCSTR lpFuncName);

BOOL AddSeDebugPrivileges(VOID);
