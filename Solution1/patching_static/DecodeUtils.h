#pragma once
#include "DecodeTypes.h"
namespace Utils {
	_insn decodeInsn(LPVOID address);
	_insn NextInstruction(_insn inst);
	bool IsInsn(LPVOID address);
	bool HasNext(_insn insn);
	x86_reg to64Bit(x86_reg reg);
	bool tryDeobfuscate(_insn insn);

	void staticDeobfuscate(char* filename);

};