#include "DecodeUtils.h"
#include <string>
#include <fstream>


#define MAX_TEXT_SECTIONS 2
CX86Disasm64 decoder;

_insn Utils::decodeInsn(LPVOID address, size_t baseAddress=0) {
	if (baseAddress == 0) {
		baseAddress = (size_t)address;
	}
	if (address == 0) {
		throw new std::exception("Cannot decode address!\n");
	}

	CS_INSN_HOLDER<_insn>* holder = decoder.Disasm(address, 20, baseAddress);
	if (holder->Count == 0) {
		throw new std::exception("Cannot decode address!\n");
	}
	return holder->Instructions(0);
}

_insn Utils::NextInstruction(_insn inst)
{
	CS_INSN_HOLDER<_insn>* holder = decoder.Disasm((void*)(inst->address + inst->size), 20, inst->address + inst->size);
	return holder->Instructions(0);
}

bool Utils::IsInsn(LPVOID address)
{
	CS_INSN_HOLDER<_insn>* holder = decoder.Disasm(address, 20, 0);
	return holder->Count;
}
bool Utils::HasNext(_insn insn)
{
	if (insn->size == 0) {
		return false;
	}
	CS_INSN_HOLDER<_insn>* holder = decoder.Disasm((void*)(insn->address + insn->size), 20, insn->address + insn->size);
	return holder->Count > 0;
}
x86_reg Utils::to64Bit(x86_reg reg)
{
	int dictionary[] = { X86_REG_INVALID,
		X86_REG_RAX, X86_REG_RAX, X86_REG_RAX, X86_REG_BH, X86_REG_BL,
		X86_REG_RBP, X86_REG_RBP, X86_REG_RBX, X86_REG_RCX, X86_REG_RCX,
		X86_REG_CS, X86_REG_RCX, X86_REG_RDX, X86_REG_RDI, X86_REG_RDI,
		X86_REG_RDX, X86_REG_DS, X86_REG_RDX, X86_REG_RAX, X86_REG_RBP,
		X86_REG_RBX, X86_REG_RCX, X86_REG_RDI, X86_REG_RDX, X86_REG_EFLAGS,
		X86_REG_RIP, X86_REG_RIZ, X86_REG_ES, X86_REG_RSI, X86_REG_RSP,
	};
	if (reg <= 30 && reg >= 0) return (x86_reg)dictionary[reg];
	else return reg;
}

bool Utils::tryDeobfuscate(_insn insn)
{
	DWORD_PTR current_addr = insn->address;
	if (std::string(insn->mnemonic) == "push") {
		if (!HasNext(insn)) return false;
		_insn calc = NextInstruction(insn);
		if (!HasNext(calc)) return false;
		auto pop = NextInstruction(calc);
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
			if (!HasNext(pop)) return false;
			_insn jmp = NextInstruction(pop);
			if (std::string(jmp->mnemonic) != "jno" && std::string(jmp->mnemonic) != "jnb" && std::string(jmp->mnemonic) != "jae")
				return false;
			if (jmp->detail->x86.operands[0].imm != jmp->address + jmp->size + 2)
				return false;
			memset((void*)current_addr, 0x90, jmp->detail->x86.operands[0].imm - current_addr);
			//printf("Found at %x\n", current_addr);
			//printf("Patch %p size %x\n", current_addr, jmp->detail->x86.operands[0].imm - current_addr);
			return true;
		}
	}
	return false;
}

void Utils::staticDeobfuscate(const char* filename)
{
	decoder.SetDetail(CS_OPT_ON);
	IMAGE_DOS_HEADER dosHeader;
	IMAGE_NT_HEADERS ntHeaders;
	std::streampos orig_address;
	char buffer[70];
	std::fstream file = std::fstream(filename, std::ios::in | std::ios::binary | std::ios::out);
	if (!file) {
		printf("File open failed!\n");
		file.close();
		return;
	}
	file.seekg(0);
	if (!file.read((char*)&dosHeader, sizeof(IMAGE_DOS_HEADER))) {
		printf("File read failed!\n");
		file.close();
		return;
	}
	file.seekg(dosHeader.e_lfanew);
	if (!file.read((char*)&ntHeaders, sizeof(IMAGE_NT_HEADERS))) { // After this read, the seeker is already at the section
																  // headers.
		printf("File read failed!\n");
		file.close();
		return;
	};
	
	IMAGE_SECTION_HEADER sectionHeader;
	IMAGE_SECTION_HEADER textSections[MAX_TEXT_SECTIONS];
	int cur = 0;
	for (int i = 0; i < ntHeaders.FileHeader.NumberOfSections &&
		cur < MAX_TEXT_SECTIONS; i++) {
		file.read((char*)&sectionHeader, sizeof(IMAGE_SECTION_HEADER));
		if (!file) break;
		printf("Section %d is named %.8s \
				in location %llx \
				sized %llx.\n", i, (char*)sectionHeader.Name, sectionHeader.PointerToRawData, sectionHeader.SizeOfRawData);
		if (strncmp((char*)sectionHeader.Name, ".text", 5) == 0) {
			textSections[cur] = sectionHeader;
			cur++;
		}
	}
	DWORD current_loc;
	int number_of_sections = cur;
	printf("Note: jmp addresses will not be printed correctly, as all instructions share the same address (buffer).\n");
	printf("Number of text sections:%d\n", cur);
	for (int i = 0; i < number_of_sections; i++) {
		current_loc = textSections[i].PointerToRawData;
		printf("Location of text section:%x\n", current_loc);
		printf("Size of text section:%x\n", textSections[i].SizeOfRawData);
		file.seekg(current_loc);
		while (current_loc - textSections[i].PointerToRawData < textSections[i].SizeOfRawData) {
			try {
				orig_address = file.tellg();
				memset(buffer, 0, 70);
				if (!file.read(buffer, 70)) {
					printf("Read failed!\n");
					break;
				}
				if (!Utils::IsInsn(buffer)) {

				}
				_insn insn = decodeInsn(buffer, (size_t)buffer);
				if (insn->address == 0) break;
				//printf("%d\n", insn->detail->x86.operands[0]);
				printf(".text::%lx %s %s\n", current_loc - textSections[i].PointerToRawData, insn->mnemonic, insn->op_str);
				if (tryDeobfuscate(insn)) {
					file.seekg(orig_address);
					file.write(buffer, strlen(buffer));
				}
				current_loc += insn->size;
				file.seekg(orig_address + std::streampos(insn->size));
			}
			catch(errNotInstruction ex){
				printf("Reached end of code section, or invalid instruction.\n");

			}
		}
	}
	file.close();
}