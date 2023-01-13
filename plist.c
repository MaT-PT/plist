#include <Windows.h>
#include <processthreadsapi.h>
// #include <psapi.h>
#include <stdio.h>
#include <tlhelp32.h>

int main(int argc, char *argv[]) {
    if (GetProcessList()) {
        return 0;
    } else {
        return 1;
    }
}

char *removeExtension(char *filename) {
    char *ext = strrchr(filename, '.');
    if (ext && !strcmp(ext, ".exe")) {
        *ext = '\0';
    }
    return filename;
}

BOOL GetProcessList() {
    HANDLE hSnapshot;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwHandleCount = 0;
    LPFILETIME lpFtCreationTime = NULL;
    LPFILETIME lpFtExitTime = NULL;
    LPFILETIME lpFtKernelTime = NULL;
    LPFILETIME lpFtUserTime = NULL;
    LPSYSTEMTIME lpStCreationTime = NULL;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed. Error: %d\n", GetLastError());
        return FALSE;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        printf("Process32First failed. Error: %d\n", GetLastError());
        CloseHandle(hSnapshot);
        return FALSE;
    }

    printf("%32s %6s %6s %6s %6s\n", "Name", "PID", "PRI", "THD", "HND");

    do {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
        if (!hProcess || hProcess == INVALID_HANDLE_VALUE) {
            // printf("OpenProcess failed. Error: %d\n", GetLastError());
        }
        if (!GetProcessHandleCount(hProcess, &dwHandleCount)) {
            // printf("GetProcessHandleCount failed. Error: %d\n", GetLastError());
        }
        // if (!GetProcessTimes(hProcess, lpFtCreationTime, lpFtExitTime, lpFtKernelTime, lpFtUserTime)) {
        //     printf("GetProcessTimes failed. Error: %d\n", GetLastError());
        // }
        if (hProcess) {
            CloseHandle(hProcess);
        }

        // FileTimeToSystemTime(lpFtCreationTime, lpStCreationTime);

        printf("%32s %6u %6u %6u %6u %08X\n", removeExtension(pe32.szExeFile), pe32.th32ProcessID, pe32.pcPriClassBase,
               pe32.cntThreads, dwHandleCount, hProcess);
        dwHandleCount = 0;

    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return TRUE;
}