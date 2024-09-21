#pragma once
#include "DecodeTypes.h"
namespace Utils {
	_insn decodeInsn(LPVOID address, size_t baseAddress);
	_insn NextInstruction(_insn inst);
	bool IsInsn(LPVOID address);
	bool HasNext(_insn insn);
	x86_reg to64Bit(x86_reg reg);
	bool tryDeobfuscate(_insn insn);
	void staticDeobfuscate(const char* filename);

};