#include "insn.h"

std::shared_ptr<PlasmaInstruction> PlasmaInstruction::next()
{
    Plasma86InsClass cs_next_inst = parser->Disasm(code, 16, m_ins->address + m_ins->size)->Instructions(0);
    std::shared_ptr<PlasmaInstruction> next_inst = std::shared_ptr<PlasmaInstruction>(new PlasmaInstruction(cs_next_inst, parser, code));
    return next_inst;
}

bool PlasmaInstruction::no_insn()
{
    return m_ins == NULL;
}
