#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>
int lenght(char * string){
    if (*string =='\0'){return 0;}
    return lenght(&string[1])+1;
}
char *pseudo(char * a){
    static int x = 0;
    if (x >=3){
        char * result = malloc((lenght(a)-4)*sizeof(char)+1);
        for (int i=0;i<=lenght(a)-5;i++){
            result[i] = a[i];
        }
        result[lenght(a)-4]='\0';
        return result;
    }
    x++;
    return a;
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

    printf("                          Name    PID    PRI    THD    HND\n");

    do {
        printf("%30s %6d %6d %6d %6d %d\n", pseudo(pe32.szExeFile),pe32.th32ProcessID,pe32.pcPriClassBase,pe32.cntThreads);
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0;
}
