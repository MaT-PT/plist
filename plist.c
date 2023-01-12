#include <windows.h>

#include <tchar.h>
#include <tlhelp32.h>

#include "plist.h"

int main(void) {
    GetProcessList();
    return 0;
}

BOOL GetProcessList() {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed. Error: %d\n", GetLastError());
        return 1;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        printf("Process32First failed. Error: %d\n", GetLastError());
        CloseHandle(hSnapshot);
        return 1;
    }

    printf("                          Name    PID    PRI    THD    HND\n");

    do {
        printf("%31s %6d %6d %6d\n", pe32.szExeFile,pe32.th32ProcessID,pe32.pcPriClassBase,pe32.cntThreads);
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0;
}