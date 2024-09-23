#pragma once
#include "Windows.h"

namespace Utils {
    LPVOID find_tls0(__int64 gamebase);
    void init_hooks(__int64 gamebase);
}