#include <dbghelp.h>
#include <stdio.h>
#include <windows.h>
#include <winternl.h>

#pragma comment(lib, "dbghelp.lib")
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
BOOL UsingNtQueryInformationProcess() {
    DWORD debugFlag = 0;
    NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugPort, &debugFlag, sizeof(debugFlag), NULL);
    return debugFlag != 0;
}

BOOL UsingCallStackConsecutiveSetjmpEx() {
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    CONTEXT context;
    STACKFRAME64 stack;
    DWORD machineType;

    ZeroMemory(&context, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    ZeroMemory(&stack, sizeof(STACKFRAME64));
    machineType = IMAGE_FILE_MACHINE_AMD64;

    // Initialize stack frame for 64-bit architecture
    stack.AddrPC.Offset = context.Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rsp;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode = AddrModeFlat;

    BOOL foundFirst = FALSE;

    while (StackWalk64(machineType, process, thread, &stack, &context, NULL, SymFunctionTableAccess64(), SymGetModuleBase64(), NULL)) {
        DWORD64 address = stack.AddrPC.Offset;
        char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)symbolBuffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        if (SymFromAddr(process, address, NULL, symbol)) {
            if (strcmp(symbol->Name, "setjmpex") == 0 && strcmp(symbol->ModuleName, "ntoskrnl.exe") == 0) {
                if (foundFirst) {
                    return TRUE;
                }
                foundFirst = TRUE;
            } else {
                foundFirst = FALSE;
            }
        }
    }
    return FALSE;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <method_number>\n", argv[0]);
        printf("Method numbers:\n");
        printf("1 - PEB method\n");
        printf("2 - CheckRemoteDebuggerPresent method\n");
        printf("3 - IsDebuggerPresent method\n");
        printf("4 - NtQueryInformationProcess method\n");
        printf("5 - Call Stack Consecutive SetjmpEx method\n");
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
            isDebuggerPresent = UsingNtQueryInformationProcess();
            break;
        case 5:
            isDebuggerPresent = UsingCallStackConsecutiveSetjmpEx();
            break;
        default:
            printf("Invalid method number. Use 1 or 2.\n");
            return FALSE;
    }

    if (isDebuggerPresent) {
        printf("The process is being debugged. Unloading debugger...\n", method);
        if (!DebugActiveProcessStop(GetCurrentProcessId())) {
            printf("Failed to unload debugger. Error code: %lu\n", GetLastError());
            return FALSE;
        }
    } else {
        printf("The process is not being debugged.\n", method);
    }

    return TRUE;
}
