#pragma once
#include "X86Disasm.hh"
#include "Windows.h"
typedef CX86InsClass _insn;

class errNotInstruction : public std::exception {

};