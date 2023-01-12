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

    printf("%30s %6s %6s %6s %6s\n", "Name", "PID", "PRI", "THD", "HND");

    do {
        printf("%30s %6u %6u %6u\n", removeExtension(pe32.szExeFile), pe32.th32ProcessID, pe32.pcPriClassBase,
               pe32.cntThreads);
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0;
}
