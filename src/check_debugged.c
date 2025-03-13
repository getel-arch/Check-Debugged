#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#pragma comment(lib, "ntdll.lib")

int main() {
    // Get the address of the PEB
    PEB* peb = (PEB*)__readgsqword(0x60);

    // Check the BeingDebugged value
    if (peb->BeingDebugged) {
        printf("The process is being debugged.\n");
    } else {
        printf("The process is not being debugged.\n");
    }

    return 0;
}
