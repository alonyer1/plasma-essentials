// patching_static.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "DecodeUtils.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Correct format: %s path/to/game.exe");
        return 1;
    }
    int inst = 0x0000FFFF;
    Utils::staticDeobfuscate(argv[1]);
    return 1;
}
