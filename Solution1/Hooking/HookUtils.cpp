
#include "HookUtils.h"

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