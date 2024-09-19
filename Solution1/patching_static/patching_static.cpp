// patching_static.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>


/*
bool tryDeobfuscate(_insn insn, DWORD_PTR current_addr) {
    if (std::string(insn->mnemonic) == "push") {
        auto calc_p = insn.next();
        if (!calc_p)
            return false;
        auto calc = *calc_p;
        auto pop_p = calc.next();
        if (!pop_p)
            return false;
        auto pop = *pop_p;
        if (std::string(calc->mnemonic) == "xor" ||
            std::string(calc->mnemonic) == "add" ||
            std::string(calc->mnemonic) == "and" ||
            std::string(calc->mnemonic) == "or" ||
            std::string(calc->mnemonic) == "sub") {
            if (std::string(pop->mnemonic) != "pop")
                return false;
            if (calc->detail->x86.operands[0].type != X86_OP_REG)
                return false;
            if (insn->detail->x86.operands[0].reg != to64Bit(calc->detail->x86.operands[0].reg) || insn->detail->x86.operands[0].reg != pop->detail->x86.operands[0].reg)
                return false;
            auto jmp = decodeInsn(current_addr + insn->size + calc->size + pop->size);
            if (!jmp)
                return false;
            if (std::string(jmp->mnemonic) != "jno" && std::string(jmp->mnemonic) != "jnb" && std::string(jmp->mnemonic) != "jae")
                return false;
            if (jmp->detail->x86.operands[0].imm != current_addr + insn->size + calc->size + pop->size + jmp->size + 2)
                return false;
            memset((file + current_addr), 0x90, jmp->detail->x86.operands[0].imm - current_addr);
            //printf("Found at %x\n", current_addr);
            //printf("Patch %p size %x\n", current_addr, jmp->detail->x86.operands[0].imm - current_addr);
            //if (jmp.operands[0].reg)
            return true;
        }
    }
    return false;
}*/
int main()
{
    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
