#pragma once

#include <Windows.h>
//

typedef struct Filter {
    DWORD dwProcessId;
    CHAR szProcessName[256];
} FILTER, *LPFILTER;

typedef struct Options {
    BOOL bShowThreads;
    BOOL bExactMatch;
} OPTIONS, *LPOPTIONS;

VOID ShowUsage(CONST LPCSTR szAppName);

BOOL GetProcessList(CONST LPOPTIONS options, CONST LPFILTER filter);

BOOL GetThreadList(CONST DWORD dwOwnerPID);
