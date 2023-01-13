#include <windows.h>

BOOL GetProcessList();
BOOL GetThreadList(DWORD dwOwnerPID);
LPSTR removeExtension(LPSTR filename);

struct Filter {
    DWORD dwProcessId;
    CHAR szProcessName[256];
};