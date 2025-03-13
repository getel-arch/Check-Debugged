#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#pragma comment(lib, "ntdll.lib")

BOOL CheckDebuggedUsingPEB() {
    
    // Get the address of the PEB
    PEB* peb = (PEB*)__readgsqword(0x60);

    return peb->BeingDebugged;
}

BOOL CheckDebuggedUsingAPI() {
    BOOL isDebuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent);
    return isDebuggerPresent;
}

int main() {
    if (argc != 2) {
        printf("Usage: %s <method_number>\n", argv[0]);
        printf("Method numbers:\n");
        printf("1 - PEB method\n");
        printf("2 - CheckRemoteDebuggerPresent method\n");
        return 1;
    }

    int method = atoi(argv[1]);
    BOOL isDebuggerPresent = FALSE;

    switch (method) {
        case 1:
            isDebuggerPresent = CheckDebuggedUsingPEB();
            break;
        case 2:
            isDebuggerPresent = CheckDebuggedUsingAPI();
            break;
        default:
            printf("Invalid method number. Use 1 or 2.\n");
            return 1;
    }

    if (isDebuggerPresent) {
        printf("The process is being debugged (method %d).\n", method);
    } else {
        printf("The process is not being debugged (method %d).\n", method);
    }

    return 0;
}
