#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1

#include <Windows.h>
//
#include <processthreadsapi.h>
#include <stdio.h>
#include "utils.h"

#pragma comment(lib, "Advapi32.lib")

CONST LPCSTR SIZE_UNITS[] = {"B", "kB", "MB", "GB", "TB"};

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
    CHAR cUnitIndex = 0;

    while (sCurrentSize > 1024) {
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

BOOL AddSeDebugPrivileges(VOID) {
    CONST DWORD dwPid = GetCurrentProcessId();

    CONST HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);
    if (!hProc) {
        PrintError("OpenProcess");
        return FALSE;
    }

    HANDLE hTok = INVALID_HANDLE_VALUE;
    if (!OpenProcessToken(hProc, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hTok)) {
        PrintError("OpenProcessToken");
        return FALSE;
    } else if (!hTok || hTok == INVALID_HANDLE_VALUE) {
        PrintError("OpenProcessToken");
        return FALSE;
    }

    // Get the value of SeDebugPrivilege from text
    LUID pDebugPriv;
    if (!LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &pDebugPriv)) {
        PrintError("LookupPrivilegeValueA");
        return FALSE;
    }

    // Adjust token privilege
    TOKEN_PRIVILEGES tokPrivs;
    tokPrivs.PrivilegeCount = 1;
    tokPrivs.Privileges[0].Luid = pDebugPriv;
    tokPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(hTok, FALSE, &tokPrivs, 0, NULL, NULL)) {
        PrintError("AdjustTokenPrivileges");
        return FALSE;
    }

    // Query token privileges to confirm whether
    BOOL bRes;
    PRIVILEGE_SET tokPrivSet;
    tokPrivSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    tokPrivSet.PrivilegeCount = 1;
    tokPrivSet.Privilege[0].Luid = pDebugPriv;
    if (!PrivilegeCheck(hTok, &tokPrivSet, &bRes)) {
        PrintError("PrivilegeCheck");
        return FALSE;
    }

    CloseHandle(hProc);
    CloseHandle(hTok);
    hTok = NULL;

    return bRes;
}
