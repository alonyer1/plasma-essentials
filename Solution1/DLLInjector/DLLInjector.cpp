#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <Shlwapi.h>

LPSTR DLL_PATH;
//#define DLL_PATH "ColorDll.dll"
#define true 1
#define false 0


BOOL dllInjector(const char* dllpath, DWORD pID);


int main(int argc, char** argv)
{

    // Create Process SUSPENDED
    PROCESS_INFORMATION pi;
    STARTUPINFOA Startup;
    ZeroMemory(&Startup, sizeof(Startup));
    ZeroMemory(&pi, sizeof(pi));
    // get the command line argument of the current process
    //LPSTR lpCmdLine = GetCommandLineA();
    if (argc < 3) {
        printf("Usage: %s prog_name dll_name\n", argv[0]);
        return 1;
    }

    LPCSTR lpCmdLine = (LPCSTR)argv[1];
    DLL_PATH = (LPSTR)argv[2];

    printf("opening process %s\n", lpCmdLine);
    if (CreateProcessA(lpCmdLine, NULL, NULL, NULL, NULL, CREATE_SUSPENDED, NULL, NULL, &Startup, &pi) == FALSE) {
        printf("couldnt open process %s\n", lpCmdLine);
        return 1;
    }

    if (!(dllInjector(DLL_PATH, pi.dwProcessId))) {
        printf("couldnt inject dll");
        return 1;
    }

    Sleep(1000); // Let the DLL finish loading
    ResumeThread(pi.hThread);
    printf("Injected dll successfully\n");
    return 0;
}

BOOL dllInjector(const char* dllpath, DWORD pID)
{
    HANDLE pHandle;
    LPVOID remoteString;
    LPVOID remoteLoadLib;

    pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

    if (!pHandle) {
        printf("couldnt open proccess with perms\n");
        return false;
    }


    remoteLoadLib = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

    remoteString = (LPVOID)VirtualAllocEx(pHandle, NULL, strlen(DLL_PATH) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(pHandle, (LPVOID)remoteString, dllpath, strlen(dllpath), NULL);
    if (NULL == CreateRemoteThread(pHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)remoteLoadLib, (LPVOID)remoteString, NULL, NULL)) {
        return false;
    }
    CloseHandle(pHandle);

    return true;
}