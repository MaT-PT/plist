#pragma once

#include <Windows.h>
//

typedef struct SizeWithUnit {
    SIZE_T sSize;
    CHAR szUnit[4];
} SIZE_WITH_UNIT, *LPSIZE_WITH_UNIT;

typedef struct TimeSpan {
    WORD wHours;
    WORD wMinutes;
    WORD wSeconds;
    WORD wMilliseconds;
} TIME_SPAN, *LPTIME_SPAN;

LPSTR RemoveExtension(LPSTR szFilename);

VOID GetSizeWithUnit(const SIZE_T sSize, LPSIZE_WITH_UNIT swuSize);

VOID TimeDeltaNsToTimeSpan(CONST ULONGLONG ullTimeNs, LPTIME_SPAN lpTimeSpan);

VOID PrintError(LPCSTR lpFuncName);

BOOL AddSeDebugPrivileges();
