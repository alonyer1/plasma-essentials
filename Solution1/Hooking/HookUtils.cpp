
#include "HookUtils.h"
#include "MinHook.h"
#include "TLS_hooks.h"
#include <iostream>
LPVOID Utils::find_tls0(__int64 gamebase) {
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)gamebase;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(gamebase + pDosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);

    bool firstTextSection = false;
    PIMAGE_SECTION_HEADER secondTextSection = nullptr;

    for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        if (strncmp((char*)pSectionHeader[i].Name, ".text", 5) == 0) {
            if (!firstTextSection) firstTextSection = true;
            else return (LPVOID)(pSectionHeader[i].VirtualAddress+gamebase);
        }
    }
}
typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect);
NtAllocateVirtualMemory_t OriginalNtAllocateVirtualMemory = nullptr;


NTSTATUS NTAPI HookedNtAllocateVirtualMemory(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect) {
    // Add your custom logic here
    //MessageBoxA(0, "Attach a debugger", "PlasmaWatch", MB_ICONINFORMATION);
    if (Protect == 0x40) {
        printf("NtAllocateVirtualMemory called! ");
        MessageBoxA(0, "Debug", "debug", MB_ICONINFORMATION);
        printf("BaseAddress:%lx, Protect:%x, Region Size:%u\n", *BaseAddress, Protect, RegionSize);
    }
    // Call the original function
    //printf("Here\n");
    return OriginalNtAllocateVirtualMemory(
        ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
}

typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(
    HANDLE             ProcessHandle,
    PVOID* BaseAddress,
    ULONG              ZeroBits,
    PULONG             RegionSize,
    ULONG              AllocationType,
    ULONG              Protect
    );


void Utils::init_hooks(__int64 gameBase) {
    MH_CreateHook(Utils::find_tls0(gameBase), &new_tls0, NULL);
    HMODULE ntdll = LoadLibraryA("ntdll");
    if (!ntdll) {
        printf("Failed to load dll!\n");
        exit(1);
    }
    //LPVOID originalNtAllocateVirtualMemory = nullptr;
    //LPVOID NtAllocateVirtualMemory = GetProcAddress(ntdll, "NtAllocateVirtualMemory");
    //MH_CreateHook(NtAllocateVirtualMemory, &anti_anti_debug, &originalNtAllocateVirtualMemory);
    MH_CreateHookApi(L"ntdll", "NtAllocateVirtualMemory", &HookedNtAllocateVirtualMemory, reinterpret_cast<LPVOID*>(&OriginalNtAllocateVirtualMemory));
    MH_EnableHook(MH_ALL_HOOKS);
}