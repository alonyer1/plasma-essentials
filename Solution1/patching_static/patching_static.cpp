// patching_static.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "DecodeUtils.h"
#include <ios>
#include <fstream>
#include <filesystem>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Correct format: %s path/to/game.exe\n", argv[0]);
        return 1;
    }

    char path_no_exe[MAX_PATH] = { 0 };
    strncpy(path_no_exe, argv[1], strlen(argv[1]) - 4);
    char dest_path[MAX_PATH];
    int current_backup = 0;
    do 
    {
        current_backup++;
        sprintf(dest_path, "%s.old%d.exe", &path_no_exe, current_backup); 
    } while (std::filesystem::exists(dest_path));

    printf("Backed up to %s\n", dest_path);
    std::ifstream  src(argv[1], std::ios::binary | std::fstream::in);
    std::ofstream  dst(dest_path, std::ios::binary | std::fstream::out);
    dst << src.rdbuf();
    dst.close();
    src.close();
    Utils::staticDeobfuscate(argv[1]);
    return 1;
}
