#include "TLS_hooks.h"
#include <Windows.h>
#include "stdio.h"
#include <stdint.h>
#include "ida-defs.h"
#include "psapi.h"
#include <winternl.h>


int isAntiCheatInitialized = 0;
long long end_shellcode = 0x0000000141381840;
long long shellcode = 0x0000000141381710;
void fix_imports(uintptr_t alloc_base);
__int64 __fastcall VEH_DecryptHandler(_EXCEPTION_POINTERS* a1);

void new_tls0()
{
    printf("Called tls_callback0\n");

    //This part will make the TLS_callback run only once.
    if (isAntiCheatInitialized)
        return;
    isAntiCheatInitialized = 1;
    //PEB hModule = *NtCurrentTeb()->ProcessEnvironmentBlock;
    auto kernel32 = GetModuleHandleA("kernel32.dll");
    char path[MAX_PATH*2]; //File path
    GetModuleFileName(NULL, (LPTSTR)path, MAX_PATH); //Get the file name
    auto v35 = CreateFileW((LPTSTR)path, 0x80000000, 1u, 0i64, 3u, 0, 0i64);
    HMODULE hModule = GetModuleHandle((LPTSTR)path); // The image base? Is this really supposed to be hardcoded? -Soup
    auto v54 = GetFileSize(v35, 0);
    auto v73 = VirtualAlloc(0i64, v54, 0x3000i64, 4i64);
    //v92 is ReadFile
    DWORD out_size;
    if (!ReadFile(v35, v73, v54, &out_size, 0) || out_size != v54)
    {
        printf("File not found!\n");
        CloseHandle(v35);
        return;
    }
    CloseHandle(v35);
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)(hModule);
    auto nt = (PIMAGE_NT_HEADERS)((char*)hModule + dos->e_lfanew); //This one causes problems for some reason.
    if (nt->Signature != 0x4550)
    {
        printf("Wrong signature\n");
        return;
    }

    auto sectionCount = nt->FileHeader.NumberOfSections + 1;
    auto sizeOfImage = nt->OptionalHeader.SizeOfImage;
    auto shellCodeSize = (uintptr_t)end_shellcode - (uintptr_t)shellcode;

    auto sectionAllocationSize = 48i64 * sectionCount;
    auto SizeOfImageAllocationSize = 12 * ((unsigned __int64)(unsigned int)sizeOfImage >> 12);

    auto v116 = shellCodeSize + sectionAllocationSize + SizeOfImageAllocationSize;

    auto v117 = (uintptr_t*)VirtualAlloc(0, v116 + 0x58, 0x3000, 0x40);
    if (!v117)
    {
        printf("Allocation failed\n");
        return;
    }
    memset(v117, 0, v116 + 0x58);
    v117[1] = (__int64)hModule;
    v117[3] = (uintptr_t)v73;


    auto v139 = CreateFileMappingW((HANDLE)-1i64, 0i64, 0x40u, 0, sizeOfImage, 0i64);
    *v117 = (uintptr_t)v139;
    auto v160 = MapViewOfFileEx(
        (HANDLE)*v117,
        0xF003Fi64,
        0i64,
        0i64,
        0i64,
        0i64);
    v117[2] = (uintptr_t)v160;
    v117[4] = (uintptr_t)v117 + 0x58;
    v117[5] = (__int64)sectionAllocationSize + (__int64)v117 + 0x58;
    auto  v185 = (__int32*)v117;
    v185[12] = (unsigned int)sizeOfImage >> 12;

    auto v180 = 0i64;
    auto v181 = 0i64;
    if (nt->FileHeader.NumberOfSections != -1)
    {
        auto v182 = v117;
        auto v183 = 0i64;
        do
        {
            auto buffer = (PMEMORY_BASIC_INFORMATION)(v183 + v182[4]);
            if (!VirtualQuery((char*)hModule + v180, buffer, 48i64))
                break;
            if (buffer->Type != 0x1000000)
                break;
            if (buffer->State == 0x10000)
                break;
            v180 += buffer->RegionSize;
            ++v181;
            v183 += 0x30i64;
        } while (v181 < nt->FileHeader.NumberOfSections + 1);
    }
    DWORD old_flags;
    VirtualProtect((LPVOID)v117[1], sizeOfImage, 64i64, &old_flags);
    memcpy((void*)v117[2], (const void*)v117[1], sizeOfImage);
    v117[5] = (uintptr_t)((char*)v117 + sectionAllocationSize + 88);
    v117[8] = (uintptr_t)UnmapViewOfFile;
    v117[9] = (uintptr_t)MapViewOfFileEx;
    v117[10] = (uintptr_t)VirtualProtect;
    auto v227 = (__int64(__fastcall*)(__int64))((char*)v117 + SizeOfImageAllocationSize + sectionAllocationSize + 0x58);
    v117[7] = (uintptr_t)v227;
    memcpy(v227, (const void*)0x0000000141381710, shellCodeSize);
    ((void(__fastcall*)(__int64*))v117[7])((__int64*)v117);
    *(uint64_t**)(0x14181D478) = v117;
    uintptr_t alloc_base = v117[2];
    fix_imports(alloc_base);
    AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER) (& VEH_DecryptHandler));
    printf("Finished tls_callback0\n");
}

void fix_imports(uintptr_t alloc_base)
{
    *(uint64_t*)(alloc_base + 0x1382158) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateSemaphoreA");
    *(uint64_t*)(alloc_base + 0x1382160) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ReleaseSemaphore");
    *(uint64_t*)(alloc_base + 0x1382168) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "OpenMutexA");
    *(uint64_t*)(alloc_base + 0x1382170) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FlushViewOfFile");
    *(uint64_t*)(alloc_base + 0x1382178) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFileSize");
    *(uint64_t*)(alloc_base + 0x1382180) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "LoadLibraryExA");
    *(uint64_t*)(alloc_base + 0x1382188) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "MoveFileExW");
    *(uint64_t*)(alloc_base + 0x1382190) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetDiskFreeSpaceExW");
    *(uint64_t*)(alloc_base + 0x1382198) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetStdHandle");
    *(uint64_t*)(alloc_base + 0x13821A0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateFileA");
    *(uint64_t*)(alloc_base + 0x13821A8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CloseHandle");
    *(uint64_t*)(alloc_base + 0x13821B0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetLastError");
    *(uint64_t*)(alloc_base + 0x13821B8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFileType");
    *(uint64_t*)(alloc_base + 0x13821C0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetStdHandle");
    *(uint64_t*)(alloc_base + 0x13821C8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "AllocConsole");
    *(uint64_t*)(alloc_base + 0x13821D0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetCommandLineA");
    *(uint64_t*)(alloc_base + 0x13821D8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFileAttributesA");
    *(uint64_t*)(alloc_base + 0x13821E0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetModuleHandleA");
    *(uint64_t*)(alloc_base + 0x13821E8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetProcAddress");
    *(uint64_t*)(alloc_base + 0x13821F0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "WideCharToMultiByte");
    *(uint64_t*)(alloc_base + 0x13821F8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ExpandEnvironmentStringsA");
    *(uint64_t*)(alloc_base + 0x1382200) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "WaitForMultipleObjects");
    *(uint64_t*)(alloc_base + 0x1382208) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "VerifyVersionInfoA");
    *(uint64_t*)(alloc_base + 0x1382210) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "VerSetConditionMask");
    *(uint64_t*)(alloc_base + 0x1382218) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "WaitForMultipleObjectsEx");
    *(uint64_t*)(alloc_base + 0x1382220) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SleepEx");
    *(uint64_t*)(alloc_base + 0x1382228) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetModuleHandleExA");
    *(uint64_t*)(alloc_base + 0x1382230) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetThreadPriority");
    *(uint64_t*)(alloc_base + 0x1382238) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "lstrcmpA");
    *(uint64_t*)(alloc_base + 0x1382240) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetSystemDirectoryW");
    *(uint64_t*)(alloc_base + 0x1382248) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "LocalAlloc");
    *(uint64_t*)(alloc_base + 0x1382250) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetEnvironmentVariableA");
    *(uint64_t*)(alloc_base + 0x1382258) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "LCMapStringW");
    *(uint64_t*)(alloc_base + 0x1382260) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CompareStringW");
    *(uint64_t*)(alloc_base + 0x1382268) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetTimeFormatW");
    *(uint64_t*)(alloc_base + 0x1382270) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetDateFormatW");
    *(uint64_t*)(alloc_base + 0x1382278) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetStringTypeW");
    *(uint64_t*)(alloc_base + 0x1382280) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SystemTimeToTzSpecificLocalTime");
    *(uint64_t*)(alloc_base + 0x1382288) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetDriveTypeW");
    *(uint64_t*)(alloc_base + 0x1382290) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FindFirstFileExW");
    *(uint64_t*)(alloc_base + 0x1382298) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetTimeZoneInformation");
    *(uint64_t*)(alloc_base + 0x13822A0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "WriteConsoleW");
    *(uint64_t*)(alloc_base + 0x13822A8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetCPInfo");
    *(uint64_t*)(alloc_base + 0x13822B0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetOEMCP");
    *(uint64_t*)(alloc_base + 0x13822B8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetACP");
    *(uint64_t*)(alloc_base + 0x13822C0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "LocalFree");
    *(uint64_t*)(alloc_base + 0x13822C8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "InitializeCriticalSection");
    *(uint64_t*)(alloc_base + 0x13822D0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "LeaveCriticalSection");
    *(uint64_t*)(alloc_base + 0x13822D8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "EnterCriticalSection");
    *(uint64_t*)(alloc_base + 0x13822E0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "DeleteCriticalSection");
    *(uint64_t*)(alloc_base + 0x13822E8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "Sleep");
    *(uint64_t*)(alloc_base + 0x13822F0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetConsoleCtrlHandler");
    *(uint64_t*)(alloc_base + 0x13822F8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetErrorMode");
    *(uint64_t*)(alloc_base + 0x1382300) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "TlsGetValue");
    *(uint64_t*)(alloc_base + 0x1382308) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "VirtualAlloc");
    *(uint64_t*)(alloc_base + 0x1382310) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "VirtualFree");
    *(uint64_t*)(alloc_base + 0x1382318) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetPriorityClass");
    *(uint64_t*)(alloc_base + 0x1382320) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetCurrentProcess");
    *(uint64_t*)(alloc_base + 0x1382328) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetThreadId");
    *(uint64_t*)(alloc_base + 0x1382330) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SuspendThread");
    *(uint64_t*)(alloc_base + 0x1382338) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ResumeThread");
    *(uint64_t*)(alloc_base + 0x1382340) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "WaitForSingleObject");
    *(uint64_t*)(alloc_base + 0x1382348) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetThreadExecutionState");
    *(uint64_t*)(alloc_base + 0x1382350) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetCurrentProcessId");
    *(uint64_t*)(alloc_base + 0x1382358) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetCurrentThreadId");
    *(uint64_t*)(alloc_base + 0x1382360) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "DuplicateHandle");
    *(uint64_t*)(alloc_base + 0x1382368) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetCurrentThread");
    *(uint64_t*)(alloc_base + 0x1382370) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateProcessA");
    *(uint64_t*)(alloc_base + 0x1382378) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "TerminateProcess");
    *(uint64_t*)(alloc_base + 0x1382380) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "RtlCaptureContext");
    *(uint64_t*)(alloc_base + 0x1382388) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "RtlLookupFunctionEntry");
    *(uint64_t*)(alloc_base + 0x1382390) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "RtlVirtualUnwind");
    *(uint64_t*)(alloc_base + 0x1382398) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateToolhelp32Snapshot");
    *(uint64_t*)(alloc_base + 0x13823A0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "Thread32First");
    *(uint64_t*)(alloc_base + 0x13823A8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "OpenThread");
    *(uint64_t*)(alloc_base + 0x13823B0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "Thread32Next");
    *(uint64_t*)(alloc_base + 0x13823B8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "OutputDebugStringA");
    *(uint64_t*)(alloc_base + 0x13823C0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetTickCount");
    *(uint64_t*)(alloc_base + 0x13823C8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetTickCount64");
    *(uint64_t*)(alloc_base + 0x13823D0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateEventA");
    *(uint64_t*)(alloc_base + 0x13823D8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetEvent");
    *(uint64_t*)(alloc_base + 0x13823E0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ResetEvent");
    *(uint64_t*)(alloc_base + 0x13823E8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "TlsSetValue");
    *(uint64_t*)(alloc_base + 0x13823F0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "TlsAlloc");
    *(uint64_t*)(alloc_base + 0x13823F8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "TlsFree");
    *(uint64_t*)(alloc_base + 0x1382400) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "RaiseException");
    *(uint64_t*)(alloc_base + 0x1382408) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateThread");
    *(uint64_t*)(alloc_base + 0x1382410) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreatePipe");
    *(uint64_t*)(alloc_base + 0x1382418) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetHandleInformation");
    *(uint64_t*)(alloc_base + 0x1382420) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetUnhandledExceptionFilter");
    *(uint64_t*)(alloc_base + 0x1382428) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateFileMappingA");
    *(uint64_t*)(alloc_base + 0x1382430) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "MapViewOfFile");
    *(uint64_t*)(alloc_base + 0x1382438) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "UnmapViewOfFile");
    *(uint64_t*)(alloc_base + 0x1382440) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "LoadLibraryA");
    *(uint64_t*)(alloc_base + 0x1382448) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FreeLibrary");
    *(uint64_t*)(alloc_base + 0x1382450) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ReadProcessMemory");
    *(uint64_t*)(alloc_base + 0x1382458) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "WriteProcessMemory");
    *(uint64_t*)(alloc_base + 0x1382460) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetExitCodeThread");
    *(uint64_t*)(alloc_base + 0x1382468) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "WriteFile");
    *(uint64_t*)(alloc_base + 0x1382470) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FlushFileBuffers");
    *(uint64_t*)(alloc_base + 0x1382478) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "PeekNamedPipe");
    *(uint64_t*)(alloc_base + 0x1382480) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ReadFile");
    *(uint64_t*)(alloc_base + 0x1382488) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "QueryPerformanceFrequency");
    *(uint64_t*)(alloc_base + 0x1382490) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "QueryPerformanceCounter");
    *(uint64_t*)(alloc_base + 0x1382498) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetSystemTime");
    *(uint64_t*)(alloc_base + 0x13824A0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "IsWow64Process");
    *(uint64_t*)(alloc_base + 0x13824A8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetNativeSystemInfo");
    *(uint64_t*)(alloc_base + 0x13824B0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetSystemInfo");
    *(uint64_t*)(alloc_base + 0x13824B8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetProcessAffinityMask");
    *(uint64_t*)(alloc_base + 0x13824C0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetProcessAffinityMask");
    *(uint64_t*)(alloc_base + 0x13824C8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetSystemTimes");
    *(uint64_t*)(alloc_base + 0x13824D0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetLogicalProcessorInformation");
    *(uint64_t*)(alloc_base + 0x13824D8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetModuleFileNameW");
    *(uint64_t*)(alloc_base + 0x13824E0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetComputerNameW");
    *(uint64_t*)(alloc_base + 0x13824E8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GlobalMemoryStatusEx");
    *(uint64_t*)(alloc_base + 0x13824F0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetVersionExA");
    *(uint64_t*)(alloc_base + 0x13824F8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateDirectoryA");
    *(uint64_t*)(alloc_base + 0x1382500) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FormatMessageA");
    *(uint64_t*)(alloc_base + 0x1382508) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FileTimeToSystemTime");
    *(uint64_t*)(alloc_base + 0x1382510) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SystemTimeToFileTime");
    *(uint64_t*)(alloc_base + 0x1382518) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFileAttributesW");
    *(uint64_t*)(alloc_base + 0x1382520) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateDirectoryW");
    *(uint64_t*)(alloc_base + 0x1382528) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FindFirstFileW");
    *(uint64_t*)(alloc_base + 0x1382530) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FindClose");
    *(uint64_t*)(alloc_base + 0x1382538) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFullPathNameW");
    *(uint64_t*)(alloc_base + 0x1382540) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateFileW");
    *(uint64_t*)(alloc_base + 0x1382548) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetFilePointer");
    *(uint64_t*)(alloc_base + 0x1382550) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetEndOfFile");
    *(uint64_t*)(alloc_base + 0x1382558) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFileInformationByHandle");
    *(uint64_t*)(alloc_base + 0x1382560) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetFileAttributesW");
    *(uint64_t*)(alloc_base + 0x1382568) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "DeleteFileW");
    *(uint64_t*)(alloc_base + 0x1382570) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FindNextFileW");
    *(uint64_t*)(alloc_base + 0x1382578) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "RemoveDirectoryW");
    *(uint64_t*)(alloc_base + 0x1382580) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CopyFileW");
    *(uint64_t*)(alloc_base + 0x1382588) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "MoveFileExA");
    *(uint64_t*)(alloc_base + 0x1382590) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetDiskFreeSpaceExA");
    *(uint64_t*)(alloc_base + 0x1382598) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFileTime");
    *(uint64_t*)(alloc_base + 0x13825A0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetFileTime");
    *(uint64_t*)(alloc_base + 0x13825A8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ReadDirectoryChangesW");
    *(uint64_t*)(alloc_base + 0x13825B0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CancelIoEx");
    *(uint64_t*)(alloc_base + 0x13825B8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetOverlappedResult");
    *(uint64_t*)(alloc_base + 0x13825C0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "TryEnterCriticalSection");
    *(uint64_t*)(alloc_base + 0x13825C8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetConsoleScreenBufferInfo");
    *(uint64_t*)(alloc_base + 0x13825D0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetConsoleTextAttribute");
    *(uint64_t*)(alloc_base + 0x13825D8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "VirtualProtect");
    *(uint64_t*)(alloc_base + 0x13825E0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetEnvironmentVariableA");
    *(uint64_t*)(alloc_base + 0x13825E8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "InitializeCriticalSectionAndSpinCount");
    *(uint64_t*)(alloc_base + 0x13825F0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetLastError");
    *(uint64_t*)(alloc_base + 0x13825F8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GlobalAlloc");
    *(uint64_t*)(alloc_base + 0x1382600) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GlobalLock");
    *(uint64_t*)(alloc_base + 0x1382608) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GlobalUnlock");
    *(uint64_t*)(alloc_base + 0x1382610) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetVersionExW");
    *(uint64_t*)(alloc_base + 0x1382618) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetEnvironmentVariableW");
    *(uint64_t*)(alloc_base + 0x1382620) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetCurrentDirectoryW");
    *(uint64_t*)(alloc_base + 0x1382628) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "OutputDebugStringW");
    *(uint64_t*)(alloc_base + 0x1382630) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateEventW");
    *(uint64_t*)(alloc_base + 0x1382638) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SwitchToThread");
    *(uint64_t*)(alloc_base + 0x1382640) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GlobalFree");
    *(uint64_t*)(alloc_base + 0x1382648) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetThreadAffinityMask");
    *(uint64_t*)(alloc_base + 0x1382650) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "MultiByteToWideChar");
    *(uint64_t*)(alloc_base + 0x1382658) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CancelIo");
    *(uint64_t*)(alloc_base + 0x1382660) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "CreateMutexA");
    *(uint64_t*)(alloc_base + 0x1382668) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ReleaseMutex");
    *(uint64_t*)(alloc_base + 0x1382670) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "EncodePointer");
    *(uint64_t*)(alloc_base + 0x1382678) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "DecodePointer");
    *(uint64_t*)(alloc_base + 0x1382680) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "IsProcessorFeaturePresent");
    *(uint64_t*)(alloc_base + 0x1382688) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ExitThread");
    *(uint64_t*)(alloc_base + 0x1382690) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "LoadLibraryExW");
    *(uint64_t*)(alloc_base + 0x1382698) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FileTimeToLocalFileTime");
    *(uint64_t*)(alloc_base + 0x13826A0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetSystemTimeAsFileTime");
    *(uint64_t*)(alloc_base + 0x13826A8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetConsoleCP");
    *(uint64_t*)(alloc_base + 0x13826B0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetConsoleMode");
    *(uint64_t*)(alloc_base + 0x13826B8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "RtlUnwindEx");
    *(uint64_t*)(alloc_base + 0x13826C0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "HeapFree");
    *(uint64_t*)(alloc_base + 0x13826C8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "HeapAlloc");
    *(uint64_t*)(alloc_base + 0x13826D0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "HeapReAlloc");
    *(uint64_t*)(alloc_base + 0x13826D8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetFileAttributesExW");
    *(uint64_t*)(alloc_base + 0x13826E0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ReadConsoleW");
    *(uint64_t*)(alloc_base + 0x13826E8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "SetFilePointerEx");
    *(uint64_t*)(alloc_base + 0x13826F0) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "RtlPcToFileHeader");
    *(uint64_t*)(alloc_base + 0x13826F8) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "ExitProcess");
    *(uint64_t*)(alloc_base + 0x1382700) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetModuleHandleExW");
    *(uint64_t*)(alloc_base + 0x1382708) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "AreFileApisANSI");
    *(uint64_t*)(alloc_base + 0x1382710) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "HeapSize");
    *(uint64_t*)(alloc_base + 0x1382718) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "UnhandledExceptionFilter");
    *(uint64_t*)(alloc_base + 0x1382720) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetStartupInfoW");
    *(uint64_t*)(alloc_base + 0x1382728) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetModuleHandleW");
    *(uint64_t*)(alloc_base + 0x1382730) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetProcessHeap");
    *(uint64_t*)(alloc_base + 0x1382738) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetModuleFileNameA");
    *(uint64_t*)(alloc_base + 0x1382740) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "GetEnvironmentStringsW");
    *(uint64_t*)(alloc_base + 0x1382748) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "FreeEnvironmentStringsW");
    *(uint64_t*)(alloc_base + 0x1382750) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "IsValidCodePage");
    *(uint64_t*)(alloc_base + 0x1382758) = (uint64_t)GetProcAddress(LoadLibraryA("KERNEL32.dll"), "IsDebuggerPresent");
    *(uint64_t*)(alloc_base + 0x1382808) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "LoadCursorA");
    *(uint64_t*)(alloc_base + 0x1382810) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "LoadIconA");
    *(uint64_t*)(alloc_base + 0x1382818) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "DefWindowProcW");
    *(uint64_t*)(alloc_base + 0x1382820) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "LoadAcceleratorsA");
    *(uint64_t*)(alloc_base + 0x1382828) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetSystemMetrics");
    *(uint64_t*)(alloc_base + 0x1382830) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "MonitorFromWindow");
    *(uint64_t*)(alloc_base + 0x1382838) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetMonitorInfoA");
    *(uint64_t*)(alloc_base + 0x1382840) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SystemParametersInfoW");
    *(uint64_t*)(alloc_base + 0x1382848) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetWindowLongPtrW");
    *(uint64_t*)(alloc_base + 0x1382850) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetMenu");
    *(uint64_t*)(alloc_base + 0x1382858) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetWindowsHookExW");
    *(uint64_t*)(alloc_base + 0x1382860) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "UnhookWindowsHookEx");
    *(uint64_t*)(alloc_base + 0x1382868) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "ClipCursor");
    *(uint64_t*)(alloc_base + 0x1382870) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "IsWindow");
    *(uint64_t*)(alloc_base + 0x1382878) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetDoubleClickTime");
    *(uint64_t*)(alloc_base + 0x1382880) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetClassLongPtrW");
    *(uint64_t*)(alloc_base + 0x1382888) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "PostMessageW");
    *(uint64_t*)(alloc_base + 0x1382890) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "RegisterRawInputDevices");
    *(uint64_t*)(alloc_base + 0x1382898) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "CreateCursor");
    *(uint64_t*)(alloc_base + 0x13828A0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetRawInputData");
    *(uint64_t*)(alloc_base + 0x13828A8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetWindowLongPtrA");
    *(uint64_t*)(alloc_base + 0x13828B0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "ShowCursor");
    *(uint64_t*)(alloc_base + 0x13828B8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "PeekMessageW");
    *(uint64_t*)(alloc_base + 0x13828C0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetWindowLongPtrW");
    *(uint64_t*)(alloc_base + 0x13828C8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetCursorPos");
    *(uint64_t*)(alloc_base + 0x13828D0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetCursor");
    *(uint64_t*)(alloc_base + 0x13828D8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "TranslateMessage");
    *(uint64_t*)(alloc_base + 0x13828E0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "DispatchMessageW");
    *(uint64_t*)(alloc_base + 0x13828E8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetCursorPos");
    *(uint64_t*)(alloc_base + 0x13828F0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetAsyncKeyState");
    *(uint64_t*)(alloc_base + 0x13828F8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "CallNextHookEx");
    *(uint64_t*)(alloc_base + 0x1382900) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetMessageTime");
    *(uint64_t*)(alloc_base + 0x1382908) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "MapVirtualKeyA");
    *(uint64_t*)(alloc_base + 0x1382910) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "MessageBoxA");
    *(uint64_t*)(alloc_base + 0x1382918) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "ReleaseDC");
    *(uint64_t*)(alloc_base + 0x1382920) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "CreateIconIndirect");
    *(uint64_t*)(alloc_base + 0x1382928) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "DestroyCursor");
    *(uint64_t*)(alloc_base + 0x1382930) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetWindowInfo");
    *(uint64_t*)(alloc_base + 0x1382938) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "RegisterClassExW");
    *(uint64_t*)(alloc_base + 0x1382940) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "ReleaseCapture");
    *(uint64_t*)(alloc_base + 0x1382948) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "UnregisterClassW");
    *(uint64_t*)(alloc_base + 0x1382950) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "PostQuitMessage");
    *(uint64_t*)(alloc_base + 0x1382958) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "BeginPaint");
    *(uint64_t*)(alloc_base + 0x1382960) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "EndPaint");
    *(uint64_t*)(alloc_base + 0x1382968) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetMenu");
    *(uint64_t*)(alloc_base + 0x1382970) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetWindowRect");
    *(uint64_t*)(alloc_base + 0x1382978) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetClientRect");
    *(uint64_t*)(alloc_base + 0x1382980) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "CallWindowProcW");
    *(uint64_t*)(alloc_base + 0x1382988) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "OpenClipboard");
    *(uint64_t*)(alloc_base + 0x1382990) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "EmptyClipboard");
    *(uint64_t*)(alloc_base + 0x1382998) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetClipboardData");
    *(uint64_t*)(alloc_base + 0x13829A0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "CloseClipboard");
    *(uint64_t*)(alloc_base + 0x13829A8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetClipboardData");
    *(uint64_t*)(alloc_base + 0x13829B0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetKeyboardLayout");
    *(uint64_t*)(alloc_base + 0x13829B8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetKeyboardLayoutList");
    *(uint64_t*)(alloc_base + 0x13829C0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetProcessDPIAware");
    *(uint64_t*)(alloc_base + 0x13829C8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "EnumDisplayDevicesA");
    *(uint64_t*)(alloc_base + 0x13829D0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "EnumDisplaySettingsExA");
    *(uint64_t*)(alloc_base + 0x13829D8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "MonitorFromPoint");
    *(uint64_t*)(alloc_base + 0x13829E0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "DestroyWindow");
    *(uint64_t*)(alloc_base + 0x13829E8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "ShowWindow");
    *(uint64_t*)(alloc_base + 0x13829F0) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetWindowLongA");
    *(uint64_t*)(alloc_base + 0x13829F8) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetWindowLongA");
    *(uint64_t*)(alloc_base + 0x1382A00) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetWindowPos");
    *(uint64_t*)(alloc_base + 0x1382A08) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "UpdateWindow");
    *(uint64_t*)(alloc_base + 0x1382A10) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetForegroundWindow");
    *(uint64_t*)(alloc_base + 0x1382A18) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetWindowPlacement");
    *(uint64_t*)(alloc_base + 0x1382A20) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "AdjustWindowRectEx");
    *(uint64_t*)(alloc_base + 0x1382A28) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "CreateWindowExA");
    *(uint64_t*)(alloc_base + 0x1382A30) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "PostMessageA");
    *(uint64_t*)(alloc_base + 0x1382A38) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetWindowLongPtrA");
    *(uint64_t*)(alloc_base + 0x1382A40) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SwitchToThisWindow");
    *(uint64_t*)(alloc_base + 0x1382A48) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetFocus");
    *(uint64_t*)(alloc_base + 0x1382A50) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "LoadStringA");
    *(uint64_t*)(alloc_base + 0x1382A58) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "RegisterDeviceNotificationW");
    *(uint64_t*)(alloc_base + 0x1382A60) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "UnregisterDeviceNotification");
    *(uint64_t*)(alloc_base + 0x1382A68) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "MessageBoxW");
    *(uint64_t*)(alloc_base + 0x1382A70) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "SetCapture");
    *(uint64_t*)(alloc_base + 0x1382A78) = (uint64_t)GetProcAddress(LoadLibraryA("USER32.dll"), "GetDC");
    *(uint64_t*)(alloc_base + 0x13820D0) = (uint64_t)GetProcAddress(LoadLibraryA("GDI32.dll"), "GetStockObject");
    *(uint64_t*)(alloc_base + 0x13820D8) = (uint64_t)GetProcAddress(LoadLibraryA("GDI32.dll"), "DeleteObject");
    *(uint64_t*)(alloc_base + 0x13820E0) = (uint64_t)GetProcAddress(LoadLibraryA("GDI32.dll"), "DeleteDC");
    *(uint64_t*)(alloc_base + 0x13820E8) = (uint64_t)GetProcAddress(LoadLibraryA("GDI32.dll"), "SetPixel");
    *(uint64_t*)(alloc_base + 0x13820F0) = (uint64_t)GetProcAddress(LoadLibraryA("GDI32.dll"), "SelectObject");
    *(uint64_t*)(alloc_base + 0x13820F8) = (uint64_t)GetProcAddress(LoadLibraryA("GDI32.dll"), "CreateCompatibleBitmap");
    *(uint64_t*)(alloc_base + 0x1382100) = (uint64_t)GetProcAddress(LoadLibraryA("GDI32.dll"), "CreateCompatibleDC");
    *(uint64_t*)(alloc_base + 0x1382C20) = (uint64_t)GetProcAddress(LoadLibraryA("ole32.dll"), "CoTaskMemAlloc");
    *(uint64_t*)(alloc_base + 0x1382C28) = (uint64_t)GetProcAddress(LoadLibraryA("ole32.dll"), "CoTaskMemFree");
    *(uint64_t*)(alloc_base + 0x1382C30) = (uint64_t)GetProcAddress(LoadLibraryA("ole32.dll"), "CoCreateFreeThreadedMarshaler");
    *(uint64_t*)(alloc_base + 0x1382C38) = (uint64_t)GetProcAddress(LoadLibraryA("ole32.dll"), "CoUninitialize");
    *(uint64_t*)(alloc_base + 0x1382C40) = (uint64_t)GetProcAddress(LoadLibraryA("ole32.dll"), "CoSetProxyBlanket");
    *(uint64_t*)(alloc_base + 0x1382C48) = (uint64_t)GetProcAddress(LoadLibraryA("ole32.dll"), "CoCreateInstance");
    *(uint64_t*)(alloc_base + 0x1382C50) = (uint64_t)GetProcAddress(LoadLibraryA("ole32.dll"), "CoInitializeEx");
    *(uint64_t*)(alloc_base + 0x1382AD0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), "WSAEventSelect");
    *(uint64_t*)(alloc_base + 0x1382AD8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), "WSACreateEvent");
    *(uint64_t*)(alloc_base + 0x1382AE0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), "WSAResetEvent");
    *(uint64_t*)(alloc_base + 0x1382AE8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)115);
    *(uint64_t*)(alloc_base + 0x1382AF0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)12);
    *(uint64_t*)(alloc_base + 0x1382AF8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), "WSAEnumNetworkEvents");
    *(uint64_t*)(alloc_base + 0x1382B00) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)116);
    *(uint64_t*)(alloc_base + 0x1382B08) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)20);
    *(uint64_t*)(alloc_base + 0x1382B10) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), "getaddrinfo");
    *(uint64_t*)(alloc_base + 0x1382B18) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)16);
    *(uint64_t*)(alloc_base + 0x1382B20) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)19);
    *(uint64_t*)(alloc_base + 0x1382B28) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)4);
    *(uint64_t*)(alloc_base + 0x1382B30) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), "WSAIoctl");
    *(uint64_t*)(alloc_base + 0x1382B38) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)9);
    *(uint64_t*)(alloc_base + 0x1382B40) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)3);
    *(uint64_t*)(alloc_base + 0x1382B48) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)23);
    *(uint64_t*)(alloc_base + 0x1382B50) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)57);
    *(uint64_t*)(alloc_base + 0x1382B58) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)52);
    *(uint64_t*)(alloc_base + 0x1382B60) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)11);
    *(uint64_t*)(alloc_base + 0x1382B68) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)14);
    *(uint64_t*)(alloc_base + 0x1382B70) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)8);
    *(uint64_t*)(alloc_base + 0x1382B78) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)10);
    *(uint64_t*)(alloc_base + 0x1382B80) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)13);
    *(uint64_t*)(alloc_base + 0x1382B88) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)1);
    *(uint64_t*)(alloc_base + 0x1382B90) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)17);
    *(uint64_t*)(alloc_base + 0x1382B98) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)21);
    *(uint64_t*)(alloc_base + 0x1382BA0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)15);
    *(uint64_t*)(alloc_base + 0x1382BA8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)7);
    *(uint64_t*)(alloc_base + 0x1382BB0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)111);
    *(uint64_t*)(alloc_base + 0x1382BB8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)0x97);
    *(uint64_t*)(alloc_base + 0x1382BC0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)18);
    *(uint64_t*)(alloc_base + 0x1382BC8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)112);
    *(uint64_t*)(alloc_base + 0x1382BD0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)2);
    *(uint64_t*)(alloc_base + 0x1382BD8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)5);
    *(uint64_t*)(alloc_base + 0x1382BE0) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), (char*)6);
    *(uint64_t*)(alloc_base + 0x1382BE8) = (uint64_t)GetProcAddress(LoadLibraryA("WS2_32.dll"), "freeaddrinfo");
    *(uint64_t*)(alloc_base + 0x1382110) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmAssociateContext");
    *(uint64_t*)(alloc_base + 0x1382118) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmGetCandidateListW");
    *(uint64_t*)(alloc_base + 0x1382120) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmGetCompositionStringW");
    *(uint64_t*)(alloc_base + 0x1382128) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmSetConversionStatus");
    *(uint64_t*)(alloc_base + 0x1382130) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmGetConversionStatus");
    *(uint64_t*)(alloc_base + 0x1382138) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmNotifyIME");
    *(uint64_t*)(alloc_base + 0x1382140) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmGetProperty");
    *(uint64_t*)(alloc_base + 0x1382148) = (uint64_t)GetProcAddress(LoadLibraryA("IMM32.dll"), "ImmGetIMEFileNameW");
    *(uint64_t*)(alloc_base + 0x1382068) = (uint64_t)GetProcAddress(LoadLibraryA("COMCTL32.dll"), (char*)89);
    *(uint64_t*)(alloc_base + 0x1382AB0) = (uint64_t)GetProcAddress(LoadLibraryA("WINMM.dll"), "timeGetDevCaps");
    *(uint64_t*)(alloc_base + 0x1382AB8) = (uint64_t)GetProcAddress(LoadLibraryA("WINMM.dll"), "timeEndPeriod");
    *(uint64_t*)(alloc_base + 0x1382AC0) = (uint64_t)GetProcAddress(LoadLibraryA("WINMM.dll"), "timeBeginPeriod");
    *(uint64_t*)(alloc_base + 0x1382078) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertFreeCertificateContext");
    *(uint64_t*)(alloc_base + 0x1382080) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertOpenStore");
    *(uint64_t*)(alloc_base + 0x1382088) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CryptStringToBinaryA");
    *(uint64_t*)(alloc_base + 0x1382090) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertCloseStore");
    *(uint64_t*)(alloc_base + 0x1382098) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertGetIssuerCertificateFromStore");
    *(uint64_t*)(alloc_base + 0x13820A0) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertCreateCertificateContext");
    *(uint64_t*)(alloc_base + 0x13820A8) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertNameToStrA");
    *(uint64_t*)(alloc_base + 0x13820B0) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertGetNameStringA");
    *(uint64_t*)(alloc_base + 0x13820B8) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CryptUnprotectData");
    *(uint64_t*)(alloc_base + 0x13820C0) = (uint64_t)GetProcAddress(LoadLibraryA("CRYPT32.dll"), "CertAddCertificateContextToStore");
    *(uint64_t*)(alloc_base + 0x1382C60) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_initialize3");
    *(uint64_t*)(alloc_base + 0x1382C68) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_register_logging_initialization");
    *(uint64_t*)(alloc_base + 0x1382C70) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_unregister_logging_handler");
    *(uint64_t*)(alloc_base + 0x1382C78) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_uninitialize");
    *(uint64_t*)(alloc_base + 0x1382C80) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_connector_create_create");
    *(uint64_t*)(alloc_base + 0x1382C88) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_strdup");
    *(uint64_t*)(alloc_base + 0x1382C90) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_issue_request2");
    *(uint64_t*)(alloc_base + 0x1382C98) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_get_default_config3");
    *(uint64_t*)(alloc_base + 0x1382CA0) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_account_anonymous_login_create");
    *(uint64_t*)(alloc_base + 0x1382CA8) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_account_logout_create");
    *(uint64_t*)(alloc_base + 0x1382CB0) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_sessiongroup_create_create");
    *(uint64_t*)(alloc_base + 0x1382CB8) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_sessiongroup_terminate_create");
    *(uint64_t*)(alloc_base + 0x1382CC0) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_sessiongroup_remove_session_create");
    *(uint64_t*)(alloc_base + 0x1382CC8) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_aux_set_speaker_level_create");
    *(uint64_t*)(alloc_base + 0x1382CD0) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_aux_set_mic_level_create");
    *(uint64_t*)(alloc_base + 0x1382CD8) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_aux_set_vad_properties_create");
    *(uint64_t*)(alloc_base + 0x1382CE0) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_connector_mute_local_mic_create");
    *(uint64_t*)(alloc_base + 0x1382CE8) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_session_set_participant_mute_for_me_create");
    *(uint64_t*)(alloc_base + 0x1382CF0) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_aux_get_render_devices_create");
    *(uint64_t*)(alloc_base + 0x1382CF8) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_aux_get_capture_devices_create");
    *(uint64_t*)(alloc_base + 0x1382D00) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_get_error_string");
    *(uint64_t*)(alloc_base + 0x1382D08) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_connector_initiate_shutdown_create");
    *(uint64_t*)(alloc_base + 0x1382D10) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_aux_set_capture_device_create");
    *(uint64_t*)(alloc_base + 0x1382D18) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_sessiongroup_set_tx_session_create");
    *(uint64_t*)(alloc_base + 0x1382D20) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_aux_get_vad_properties_create");
    *(uint64_t*)(alloc_base + 0x1382D28) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_sessiongroup_get_stats_create");
    *(uint64_t*)(alloc_base + 0x1382D30) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_get_message");
    *(uint64_t*)(alloc_base + 0x1382D38) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "destroy_evt");
    *(uint64_t*)(alloc_base + 0x1382D40) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "destroy_resp");
    *(uint64_t*)(alloc_base + 0x1382D48) = (uint64_t)GetProcAddress(LoadLibraryA("vivoxsdk_x64.dll"), "vx_req_sessiongroup_add_session_create");
    *(uint64_t*)(alloc_base + 0x1382BF8) = (uint64_t)GetProcAddress(LoadLibraryA("d3d11.dll"), "D3D11CreateDevice");
    *(uint64_t*)(alloc_base + 0x1382C08) = (uint64_t)GetProcAddress(LoadLibraryA("dxgi.dll"), "CreateDXGIFactory");
    *(uint64_t*)(alloc_base + 0x1382C10) = (uint64_t)GetProcAddress(LoadLibraryA("dxgi.dll"), "CreateDXGIFactory1");
    *(uint64_t*)(alloc_base + 0x1382768) = (uint64_t)GetProcAddress(LoadLibraryA("MSACM32.dll"), "acmStreamOpen");
    *(uint64_t*)(alloc_base + 0x1382770) = (uint64_t)GetProcAddress(LoadLibraryA("MSACM32.dll"), "acmStreamSize");
    *(uint64_t*)(alloc_base + 0x1382778) = (uint64_t)GetProcAddress(LoadLibraryA("MSACM32.dll"), "acmStreamConvert");
    *(uint64_t*)(alloc_base + 0x1382780) = (uint64_t)GetProcAddress(LoadLibraryA("MSACM32.dll"), "acmStreamPrepareHeader");
    *(uint64_t*)(alloc_base + 0x13827F8) = (uint64_t)GetProcAddress(LoadLibraryA("SHLWAPI.dll"), "PathIsRelativeA");
    *(uint64_t*)(alloc_base + 0x13827A8) = (uint64_t)GetProcAddress(LoadLibraryA("PSAPI.DLL"), "GetProcessMemoryInfo");
    *(uint64_t*)(alloc_base + 0x1382000) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptGenRandom");
    *(uint64_t*)(alloc_base + 0x1382008) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptAcquireContextW");
    *(uint64_t*)(alloc_base + 0x1382010) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptDestroyHash");
    *(uint64_t*)(alloc_base + 0x1382018) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptHashData");
    *(uint64_t*)(alloc_base + 0x1382020) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptCreateHash");
    *(uint64_t*)(alloc_base + 0x1382028) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptGetHashParam");
    *(uint64_t*)(alloc_base + 0x1382030) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptReleaseContext");
    *(uint64_t*)(alloc_base + 0x1382038) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "CryptAcquireContextA");
    *(uint64_t*)(alloc_base + 0x1382040) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "RegCloseKey");
    *(uint64_t*)(alloc_base + 0x1382048) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "RegQueryValueExW");
    *(uint64_t*)(alloc_base + 0x1382050) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "RegOpenKeyExW");
    *(uint64_t*)(alloc_base + 0x1382058) = (uint64_t)GetProcAddress(LoadLibraryA("ADVAPI32.dll"), "GetUserNameW");
    *(uint64_t*)(alloc_base + 0x13827B8) = (uint64_t)GetProcAddress(LoadLibraryA("SHELL32.dll"), "ShellExecuteA");
    *(uint64_t*)(alloc_base + 0x13827C0) = (uint64_t)GetProcAddress(LoadLibraryA("SHELL32.dll"), "SHFileOperationW");
    *(uint64_t*)(alloc_base + 0x13827C8) = (uint64_t)GetProcAddress(LoadLibraryA("SHELL32.dll"), "DragAcceptFiles");
    *(uint64_t*)(alloc_base + 0x13827D0) = (uint64_t)GetProcAddress(LoadLibraryA("SHELL32.dll"), "DragQueryFileA");
    *(uint64_t*)(alloc_base + 0x13827D8) = (uint64_t)GetProcAddress(LoadLibraryA("SHELL32.dll"), "DragFinish");
    *(uint64_t*)(alloc_base + 0x13827E0) = (uint64_t)GetProcAddress(LoadLibraryA("SHELL32.dll"), "SHGetFolderPathW");
    *(uint64_t*)(alloc_base + 0x13827E8) = (uint64_t)GetProcAddress(LoadLibraryA("SHELL32.dll"), "SHGetFolderPathA");
    *(uint64_t*)(alloc_base + 0x1382790) = (uint64_t)GetProcAddress(LoadLibraryA("OLEAUT32.dll"), (char*)2);
    *(uint64_t*)(alloc_base + 0x1382798) = (uint64_t)GetProcAddress(LoadLibraryA("OLEAUT32.dll"), (char*)6);
    *(uint64_t*)(alloc_base + 0x1382A88) = (uint64_t)GetProcAddress(LoadLibraryA("WINHTTP.dll"), "WinHttpGetProxyForUrl");
    *(uint64_t*)(alloc_base + 0x1382A90) = (uint64_t)GetProcAddress(LoadLibraryA("WINHTTP.dll"), "WinHttpOpen");
    *(uint64_t*)(alloc_base + 0x1382A98) = (uint64_t)GetProcAddress(LoadLibraryA("WINHTTP.dll"), "WinHttpCloseHandle");
    *(uint64_t*)(alloc_base + 0x1382AA0) = (uint64_t)GetProcAddress(LoadLibraryA("WINHTTP.dll"), "WinHttpGetIEProxyConfigForCurrentUser");
}

void anti_anti_debug()
{
    MessageBoxA(0, "Attach debugger.", "plasmawatch", MB_ICONINFORMATION);
}

__int64 __fastcall VEH_DecryptHandler(_EXCEPTION_POINTERS* a1)
{
    PEXCEPTION_RECORD ExceptionRecord; // rcx
    unsigned __int64 v3; // r15
    unsigned int v4; // edi
    __int64 v5; // rbp
    IMAGE_DOS_HEADER* v6; // rdx
    IMAGE_NT_HEADERS* v7; // r10
    struct _PEB* v8; // r9
    struct _PEB_LDR_DATA* Ldr; // r9

    __int64 result; // rax
    int v15; // edx
    BYTE* i; // rcx
    unsigned __int64 v17; // rbx
    __int64 v18; // rdx
    __int64 v19; // r14
    __int64 v20; // r12
    __int64* v21; // rcx
    __int64 v22; // rdi
    IMAGE_NT_HEADERS* v23; // rdx
    __int64 VirtualAddress; // rax
    __int64 v25; // r8
    uint32_t* v26; // r11
    int v27; // eax
    uint16_t* v28; // r9
    unsigned __int64 v29; // rax
    __int64 v30; // r10
    uint16_t* v31; // rdx
    __int64 v32; // rdx
    __int64 v33; // r10
    __int64 v34; // r8
    unsigned __int64 v35; // r9
    unsigned __int8 v36; // al
    char* v37; // rax
    void* v38; // rbx
    uint32_t* v39; // rcx
    __int64 v40; // rax
    __int64 v41; // r8
    uint32_t* v42; // r10
    int v43; // eax
    uint16_t* v44; // r9
    unsigned __int64 v45; // rax
    __int64 v46; // r11
    uint16_t* v47; // rdx
    __int64 v48; // rbx
    unsigned int v49; // edx
    int v50; // eax
    __int64 v51; // rcx
    int v52; // [rsp+70h] [rbp+18h] BYREF

    printf("HIIII\n");
    auto qword_14181D478 = *(uint64_t**)0x14181D478;
    v37 = (char*)(*((uint64_t**)0x14181D478));
    ExceptionRecord = a1->ExceptionRecord;
    v3 = ExceptionRecord->ExceptionInformation[1] & 0xFFFFFFFFFFFFF000ui64;
    v4 = (ExceptionRecord->ExceptionInformation[1] & 0xFFFFF000) - *((uint32_t*)qword_14181D478 + 2);
    printf("[EXC] Code %X Addr %llX\n", ExceptionRecord->ExceptionCode, ExceptionRecord->ExceptionAddress);
    Sleep(1000);
    /*         if (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
                 return -1;*/
    if (ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT)
    {
        __debugbreak();
        //printf("Return addr %llX\n", __builtin_return_address(0));

        Sleep(1000);
        return 0;
    }
    if (ExceptionRecord->ExceptionCode != 0xC0000005)
        return 0;

    ((void(__fastcall*)(unsigned __int64, __int64, __int64, int*))(v37+80))(v3, 0x1000i64, 0x20i64, &v52);
    return -1;
}