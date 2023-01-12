#include <Windows.h>
#include <stdio.h>
#include <tlhelp32.h>

char *removeExtension(char *filename) {
    char *ext = strrchr(filename, '.');
    if (ext && !strcmp(ext, ".exe")) {
        *ext = '\0';
    }
    return filename;
}

int main() {
    HANDLE hSnapshot;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwHandleCount = 0;

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

    printf("%32s %6s %6s %6s %6s\n", "Name", "PID", "PRI", "THD", "HND");

    do {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
        GetProcessHandleCount(hProcess, &dwHandleCount);
        if (hProcess) {
            CloseHandle(hProcess);
        }

        printf("%32s %6u %6u %6u %6u\n", removeExtension(pe32.szExeFile), pe32.th32ProcessID, pe32.pcPriClassBase,
               pe32.cntThreads, dwHandleCount);
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0;
}
