#include <windows.h>

typedef struct Filter {
    DWORD dwProcessId;
    CHAR szProcessName[256];
} FILTER, *LPFILTER;


BOOL GetProcessList(CONST BOOL bShowThreads, CONST LPFILTER filter);

BOOL GetThreadList(CONST DWORD dwOwnerPID);

LPSTR RemoveExtension(LPSTR filename);
