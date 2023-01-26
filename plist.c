#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1

#include <Windows.h>
//
#include <Psapi.h>
#include <TlHelp32.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysinfoapi.h>
#include "plist.h"
#include "utils.h"

int main(IN CONST DWORD argc, IN CONST LPCSTR argv[]) {
    if (!GrantSeDebugPrivilege()) {
        printf("Warning: Please run as administrator to get all infos.\n\n");
        // return 1;
    }

    OPTIONS options;
    FILTER filter;
    LPCSTR lpEnd;
    DWORD dwRes;

    // Set default options
    options.bShowThreads = FALSE;
    options.bShowMemory = FALSE;
    options.bExactMatch = FALSE;

    // Set default filter
    filter.dwProcessId = MAXDWORD;
    filter.szProcessName[0] = '\0';
    filter.nProcessNameLength = 0;

    // Manage command-line arguments
    for (DWORD i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h")) {
            ShowUsage(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-d")) {
            options.bShowThreads = TRUE;
        } else if (!strcmp(argv[i], "-m")) {
            options.bShowMemory = TRUE;
        } else if (!strcmp(argv[i], "-e")) {
            options.bExactMatch = TRUE;
        } else {
            if (argv[i][0] == '-') {
                printf("Invalid argument: %s\n", argv[i]);
                ShowUsage(argv[0]);
                return 1;
            }

            if (filter.dwProcessId != MAXDWORD || filter.nProcessNameLength) {
                printf("Only one process name or PID can be specified.\n");
                return 1;
            }

            dwRes = strtoul(argv[i], &lpEnd, 10);

            if (lpEnd == argv[i] || lpEnd[0] != '\0' || errno == ERANGE) {
                strcpy_s(filter.szProcessName, sizeof(filter.szProcessName), argv[i]);
                filter.nProcessNameLength = strlen(filter.szProcessName);
            } else {
                filter.dwProcessId = dwRes;
            }
        }
    }

    if (GetProcessList(&options, &filter)) {
        return 0;
    } else {
        return 1;
    }
}

VOID ShowUsage(IN CONST LPCSTR szAppName) {
    printf("Usage: %s [-h] [-d] [-m] [-e] [name|PID]\n", szAppName);
    printf("    -d          Show thread detail.\n");
    printf("    -m          Show memory detail.\n");
    printf("    -e          Match process name exactly.\n");
    printf("    name        Show information about processes that begin with the name specified.\n");
    printf("    PID         Show information about specified process.\n");
}

BOOL CheckFilter(IN CONST LPFILTER filter, IN CONST BOOL bExactMatch, IN CONST LPCSTR szProcessName,
                 IN CONST DWORD dwProcessId) {
    if (filter->dwProcessId != MAXDWORD && dwProcessId != filter->dwProcessId) {
        return FALSE;
    }

    if (filter->szProcessName[0] != '\0') {
        if (bExactMatch) {
            if (_stricmp(szProcessName, filter->szProcessName)) {
                return FALSE;
            }
        } else {
            if (_strnicmp(szProcessName, filter->szProcessName, filter->nProcessNameLength)) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL GetProcessList(IN CONST LPOPTIONS options, IN CONST LPFILTER filter) {
    BOOL bFirst = TRUE;
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    HANDLE hProcess = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    DWORD dwHandleCount;
    PROCESS_MEMORY_COUNTERS pmc;
    SIZE_WITH_UNIT swuPagefileUsage, swuPeakPagefileUsage, swuWorkingSetSize, swuQuotaPagedPoolUsage,
        swuQuotaNonPagedPoolUsage;
    LPSTR szFilenameNoExt;

    FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUserTime, ftCurrentTime, ftIdleTime;
    ULARGE_INTEGER uliCreationTime, uliKernelTime, uliUserTime, uliCurrentTime, uliIdleTime;
    TIME_SPAN tsProcessAge, tsCpuTime;

    CHAR szProcessPath[MAX_PATH];
    DWORD dwProcessPathLength;

    CONST ULONGLONG ullTickCount = GetTickCount64();

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (!hSnapshot || hSnapshot == INVALID_HANDLE_VALUE) {
        PrintError("CreateToolhelp32Snapshot");
        return FALSE;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        PrintError("Process32First");
        CloseHandle(hSnapshot);
        return FALSE;
    }

    // Get current time
    GetSystemTimeAsFileTime(&ftCurrentTime);
    uliCurrentTime.LowPart = ftCurrentTime.dwLowDateTime;
    uliCurrentTime.HighPart = ftCurrentTime.dwHighDateTime;

    do {
        if (pe32.th32ProcessID == 0) {
            // Process ID 0 is System Idle Process, change name to "Idle" instead of "[System Process]"
            szFilenameNoExt = _strdup("Idle");
        } else {
            szFilenameNoExt = RemoveExtension(pe32.szExeFile);
        }

        // Skip if process doesn't match filter
        if (!CheckFilter(filter, options->bExactMatch, szFilenameNoExt, pe32.th32ProcessID)) {
            continue;
        }

        if (options->bShowThreads) {
            // Don't print empty line before first process
            if (!bFirst) {
                printf("\n");
            }
            printf("%s %u:\n", szFilenameNoExt, pe32.th32ProcessID);

            GetThreadList(pe32.th32ProcessID);
        } else {
            if (bFirst) {
                if (options->bShowMemory) {
                    printf("%-32s %6s %9s %9s %9s %9s %9s %9s\n", "Name", "PID", "WS", "Priv", "Priv Pk", "Faults",
                           "NonP", "Page");
                } else {
                    printf("%-32s %6s %6s %6s %6s %9s %15s %15s  %s\n", "Name", "PID", "Pri", "Thd", "Hnd", "Priv",
                           "CPU Time", "Elapsed Time", "Image Path");
                }
            }

            // Reset variables
            dwHandleCount = 0;
            ZeroMemory(&pmc, sizeof(pmc));
            ZeroMemory(&tsProcessAge, sizeof(tsProcessAge));
            ZeroMemory(&tsCpuTime, sizeof(tsCpuTime));
            szProcessPath[0] = '\0';

            hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);

            if (!hProcess || hProcess == INVALID_HANDLE_VALUE) {
                // PrintError("OpenProcess");
            } else {
                if (!GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    PrintError("GetProcessMemoryInfo");
                }

                if (!options->bShowMemory) {
                    if (!GetProcessHandleCount(hProcess, &dwHandleCount)) {
                        PrintError("GetProcessHandleCount");
                    }

                    if (GetProcessTimes(hProcess, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime)) {
                        uliCreationTime.LowPart = ftCreationTime.dwLowDateTime;
                        uliCreationTime.HighPart = ftCreationTime.dwHighDateTime;
                        uliKernelTime.LowPart = ftKernelTime.dwLowDateTime;
                        uliKernelTime.HighPart = ftKernelTime.dwHighDateTime;
                        uliUserTime.LowPart = ftUserTime.dwLowDateTime;
                        uliUserTime.HighPart = ftUserTime.dwHighDateTime;

                        // Time since process creation
                        TimeDeltaNsToTimeSpan(uliCurrentTime.QuadPart - uliCreationTime.QuadPart, &tsProcessAge);
                        // CPU time = kernel time + user time
                        TimeDeltaNsToTimeSpan(uliKernelTime.QuadPart + uliUserTime.QuadPart, &tsCpuTime);
                    } else {
                        PrintError("GetProcessTimes");
                    }
                }

                dwProcessPathLength = sizeof(szProcessPath);
                QueryFullProcessImageName(hProcess, 0, szProcessPath, &dwProcessPathLength);

                CloseHandle(hProcess);
            }

            GetSizeWithUnit(pmc.PagefileUsage, &swuPagefileUsage);

            if (options->bShowMemory) {
                GetSizeWithUnit(pmc.PeakPagefileUsage, &swuPeakPagefileUsage);
                GetSizeWithUnit(pmc.WorkingSetSize, &swuWorkingSetSize);
                GetSizeWithUnit(pmc.QuotaPagedPoolUsage, &swuQuotaPagedPoolUsage);
                GetSizeWithUnit(pmc.QuotaNonPagedPoolUsage, &swuQuotaNonPagedPoolUsage);

                printf("%-32s %6u %6llu %-2s %6llu %-2s %6llu %-2s %9u %6llu %-2s %6llu %-2s\n", szFilenameNoExt,
                       pe32.th32ProcessID, swuWorkingSetSize.sSize, swuWorkingSetSize.szUnit, swuPagefileUsage.sSize,
                       swuPagefileUsage.szUnit, swuPeakPagefileUsage.sSize, swuPeakPagefileUsage.szUnit,
                       pmc.PageFaultCount, swuQuotaNonPagedPoolUsage.sSize, swuQuotaNonPagedPoolUsage.szUnit,
                       swuQuotaPagedPoolUsage.sSize, swuQuotaPagedPoolUsage.szUnit);
            } else {
                if (pe32.th32ProcessID == 0) {
                    if (!GetSystemTimes(&ftIdleTime, NULL, NULL)) {
                        PrintError("GetSystemTimes");
                    }
                    uliIdleTime.LowPart = ftIdleTime.dwLowDateTime;
                    uliIdleTime.HighPart = ftIdleTime.dwHighDateTime;

                    TimeDeltaNsToTimeSpan(uliIdleTime.QuadPart, &tsCpuTime);
                    TimeDeltaNsToTimeSpan(ullTickCount * 10000, &tsProcessAge);
                }

                printf("%-32s %6u %6u %6u %6u %6llu %-2s %5hu:%02hu:%02hu.%03hu %5hu:%02hu:%02hu.%03hu  %s\n",
                       szFilenameNoExt, pe32.th32ProcessID, pe32.pcPriClassBase, pe32.cntThreads, dwHandleCount,
                       swuPagefileUsage.sSize, swuPagefileUsage.szUnit, tsCpuTime.wHours, tsCpuTime.wMinutes,
                       tsCpuTime.wSeconds, tsCpuTime.wMilliseconds, tsProcessAge.wHours, tsProcessAge.wMinutes,
                       tsProcessAge.wSeconds, tsProcessAge.wMilliseconds, szProcessPath);
            }
        }

        if (bFirst) {
            bFirst = FALSE;
        }

        free(szFilenameNoExt);

    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (bFirst) {
        if (filter->dwProcessId != MAXDWORD) {
            printf("Process with PID %u was not found.\n", filter->dwProcessId);
        } else if (filter->nProcessNameLength) {
            printf("Process %s '%s' was not found.\n", options->bExactMatch ? "with exact name" : "name starting with",
                   filter->szProcessName);
        } else {
            printf("No processes found, this is an error.\n");
        }
    }

    return TRUE;
}

BOOL GetThreadList(IN CONST DWORD dwOwnerPID) {
    HANDLE hThreadSnapshot = INVALID_HANDLE_VALUE;
    HANDLE hThread = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUserTime, ftCurrentTime;
    ULARGE_INTEGER uliCreationTime, uliKernelTime, uliUserTime, uliCurrentTime;
    TIME_SPAN tsThreadAge, tsKernelTime, tsUserTime;

    hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (!hThreadSnapshot || hThreadSnapshot == INVALID_HANDLE_VALUE) {
        PrintError("CreateToolhelp32Snapshot");
        return FALSE;
    }

    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnapshot, &te32)) {
        PrintError("Thread32First");
        CloseHandle(hThreadSnapshot);
        return FALSE;
    }

    GetSystemTimeAsFileTime(&ftCurrentTime);
    uliCurrentTime.LowPart = ftCurrentTime.dwLowDateTime;
    uliCurrentTime.HighPart = ftCurrentTime.dwHighDateTime;

    printf("%5s %3s %15s %15s %15s\n", "TID", "Pri", "User Time", "Kernel Time", "Elapsed Time");

    do {
        if (te32.th32OwnerProcessID == dwOwnerPID) {
            tsThreadAge.wHours = tsThreadAge.wMinutes = tsThreadAge.wSeconds = tsThreadAge.wMilliseconds = 0;
            tsKernelTime.wHours = tsKernelTime.wMinutes = tsKernelTime.wSeconds = tsKernelTime.wMilliseconds = 0;
            tsUserTime.wHours = tsUserTime.wMinutes = tsUserTime.wSeconds = tsUserTime.wMilliseconds = 0;

            hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, te32.th32ThreadID);

            if (!hThread || hThread == INVALID_HANDLE_VALUE) {
                // PrintError("OpenThread");
            } else {
                if (GetThreadTimes(hThread, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime)) {
                    uliCreationTime.LowPart = ftCreationTime.dwLowDateTime;
                    uliCreationTime.HighPart = ftCreationTime.dwHighDateTime;
                    uliKernelTime.LowPart = ftKernelTime.dwLowDateTime;
                    uliKernelTime.HighPart = ftKernelTime.dwHighDateTime;
                    uliUserTime.LowPart = ftUserTime.dwLowDateTime;
                    uliUserTime.HighPart = ftUserTime.dwHighDateTime;

                    // Time since thread creation
                    TimeDeltaNsToTimeSpan(uliCurrentTime.QuadPart - uliCreationTime.QuadPart, &tsThreadAge);

                    TimeDeltaNsToTimeSpan(uliKernelTime.QuadPart, &tsKernelTime);
                    TimeDeltaNsToTimeSpan(uliUserTime.QuadPart, &tsUserTime);
                } else {
                    PrintError("GetThreadTimes");
                }

                CloseHandle(hThread);
            }

            printf("%5u %3u %5hu:%02hu:%02hu.%03hu %5hu:%02hu:%02hu.%03hu %5hu:%02hu:%02hu.%03hu\n", te32.th32ThreadID,
                   te32.tpBasePri + te32.tpDeltaPri, tsUserTime.wHours, tsUserTime.wMinutes, tsUserTime.wSeconds,
                   tsUserTime.wMilliseconds, tsKernelTime.wHours, tsKernelTime.wMinutes, tsKernelTime.wSeconds,
                   tsKernelTime.wMilliseconds, tsThreadAge.wHours, tsThreadAge.wMinutes, tsThreadAge.wSeconds,
                   tsThreadAge.wMilliseconds);
        }

    } while (Thread32Next(hThreadSnapshot, &te32));

    CloseHandle(hThreadSnapshot);
    return TRUE;
}