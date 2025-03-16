#include <stdio.h>
#include <windows.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

// Checks the BeingDebugged flag of the PEB
BOOL UsingPEB() {
    
    // Get the address of the PEB
    PEB* peb = (PEB*)__readgsqword(0x60);
    return peb->BeingDebugged;
}

// Checks using the CheckRemoteDebuggerPresent winapi call
BOOL UsingCheckRemoteDebuggerPresent() {
    BOOL isDebuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent);
    return isDebuggerPresent;
}

// Checks using the IsDebuggerPresent winapi call
BOOL UsingIsDebuggerPresent() {
    return IsDebuggerPresent();
}

// Checks using the NtQueryInformationProcess winapi call and passing the ProcessInformationClass 'ProcessDebugPort'
// which returns nozero value when the process is being debugged by a ring 3 debugger
BOOL UsingNtQueryInformationProcessDebugPort() {
    DWORD debugFlag = 0;
    NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugPort, &debugFlag, sizeof(debugFlag), NULL);
    return debugFlag != 0;
}

BOOL UsingNtQueryInformationProcessDebugFlags() {
    DWORD debugFlag = 0;
    NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugFlags, &debugFlag, sizeof(debugFlag), NULL);
    return debugFlag == 0;
}

int main(int argc, char* argv[]) {
    printf("%d\n", GetCurrentThreadId());
    if (argc != 2) {
        printf("Usage: %s <method_number>\n", argv[0]);
        printf("Method numbers:\n");
        printf("1 - PEB method\n");
        printf("2 - CheckRemoteDebuggerPresent method\n");
        printf("3 - IsDebuggerPresent method\n");
        printf("4 - NtQueryInformationProcess ProcessDebugPort method\n");
        printf("5 - NtQueryInformationProcess ProcessDebugFlags method\n");
        return FALSE;
    }

    int method = atoi(argv[1]);
    BOOL isDebuggerPresent = FALSE;

    switch (method) {
        case 1:
            isDebuggerPresent = UsingPEB();
            break;
        case 2:
            isDebuggerPresent = UsingCheckRemoteDebuggerPresent();
            break;
        case 3:
            isDebuggerPresent = UsingIsDebuggerPresent();
            break;
        case 4:
            isDebuggerPresent = UsingNtQueryInformationProcessDebugPort();
            break;
        case 5:
            isDebuggerPresent = UsingNtQueryInformationProcessDebugFlags();
            break;
        default:
            printf("Invalid method number. Use 1 or 2.\n");
            return FALSE;
    }

    if (isDebuggerPresent) {
        printf("The process is being debugged.\n", method);
    } else {
        printf("The process is not being debugged.\n", method);
    }

    return TRUE;
}
