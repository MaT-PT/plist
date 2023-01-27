#pragma once

#include <Windows.h>
//

typedef struct Filter {
    DWORD dwProcessId;
    CHAR szProcessName[256];
    SIZE_T nProcessNameLength;
} FILTER, *LPFILTER;

typedef struct Options {
    BOOL bShowThreads;
    BOOL bShowMemory;
    BOOL bExactMatch;
} OPTIONS, *LPOPTIONS;

VOID ShowUsage(IN CONST LPCSTR szAppName);

BOOL CheckFilter(IN CONST LPFILTER filter, IN CONST BOOL bExactMatch, IN CONST LPCSTR szProcessName,
                 IN CONST DWORD dwProcessId);

BOOL GetProcessList(IN CONST LPOPTIONS options, IN CONST LPFILTER filter);

BOOL GetThreadList(IN CONST DWORD dwOwnerPID, IN CONST PULARGE_INTEGER uliCurrentTime);
