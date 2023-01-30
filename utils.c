#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1

#include <Windows.h>
//
#include <processthreadsapi.h>
#include <stdio.h>
#include "utils.h"

#pragma comment(lib, "Advapi32.lib")

CONST LPCSTR SIZE_UNITS[] = {"B", "kB", "MB", "GB", "TB"};
CONST DWORD SIZE_UNITS_COUNT = sizeof(SIZE_UNITS) / sizeof(SIZE_UNITS[0]);

LPSTR RemoveExtension(IN CONST LPCSTR szFilename) {
    CONST LPSTR szFilenameCopy = _strdup(szFilename);
    CONST LPSTR szExt = strrchr(szFilenameCopy, '.');

    if (szExt && !_stricmp(szExt, ".exe")) {
        szExt[0] = '\0';
    }

    return szFilenameCopy;
}

VOID GetSizeWithUnit(IN CONST SIZE_T sSize, OUT CONST LPSIZE_WITH_UNIT swuSize) {
    SIZE_T sCurrentSize = sSize;
    WORD cUnitIndex = 0;

    while (sCurrentSize > 1024 && cUnitIndex < SIZE_UNITS_COUNT - 1) {
        sCurrentSize /= 1024;
        cUnitIndex++;
    }

    swuSize->sSize = sCurrentSize;
    strcpy_s(swuSize->szUnit, sizeof(swuSize->szUnit), SIZE_UNITS[cUnitIndex]);
}

VOID TimeDeltaNsToTimeSpan(IN CONST ULONGLONG ullTimeNs, OUT CONST LPTIME_SPAN lpTimeSpan) {
    CONST ULONGLONG ullTimeMs = ullTimeNs / 10000;
    CONST ULONGLONG ullTimeSec = ullTimeMs / 1000;
    CONST ULONGLONG ullTimeMin = ullTimeSec / 60;

    lpTimeSpan->wHours = (DWORD)(ullTimeMin / 60);
    lpTimeSpan->wMinutes = ullTimeMin % 60;
    lpTimeSpan->wSeconds = ullTimeSec % 60;
    lpTimeSpan->wMilliseconds = ullTimeMs % 1000;
}

VOID PrintError(IN CONST LPCSTR lpFuncName) {
    // Get the latest error ID
    CONST DWORD dwErrId = GetLastError();
    printf("[ERR:%d] %s: ", dwErrId, lpFuncName);

    // Pring the error message based on the response
    if (dwErrId) {
        LPSTR lpMsgBuf;
        CONST DWORD dwRes =
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, dwErrId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
        if (dwRes) {
            printf("%s\n", lpMsgBuf);
            LocalFree(lpMsgBuf);
        } else {
            printf("Unknown error\n");
        }
    } else {
        printf("Something went wrong\n");
    }
}

BOOL GrantSeDebugPrivilege(VOID) {
    CONST HANDLE hProc = GetCurrentProcess();

    HANDLE hTok = INVALID_HANDLE_VALUE;
    if (!OpenProcessToken(hProc, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hTok) || !hTok ||
        hTok == INVALID_HANDLE_VALUE) {
        PrintError("OpenProcessToken");
        return FALSE;
    }

    // Get the value of SeDebugPrivilege from its name
    LUID pDebugPriv;
    if (!LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &pDebugPriv)) {
        PrintError("LookupPrivilegeValueA");
        CloseHandle(hTok);
        return FALSE;
    }

    // Adjust token privilege
    TOKEN_PRIVILEGES tokPrivs;
    tokPrivs.PrivilegeCount = 1;
    tokPrivs.Privileges[0].Luid = pDebugPriv;
    tokPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(hTok, FALSE, &tokPrivs, 0, NULL, NULL) || GetLastError() != ERROR_SUCCESS) {
        PrintError("AdjustTokenPrivileges");
        CloseHandle(hTok);
        return FALSE;
    }

    CloseHandle(hTok);

    return TRUE;
}
