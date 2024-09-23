#pragma once
#include "vadefs.h"

void new_tls0();
void fix_imports(uintptr_t alloc_base);
void anti_anti_debug();