#pragma once

#include "afx/elf.h"

namespace afx {
    bool shvalid(int index) {
        switch (index) {
        case SHN_UNDEF:
        case SHN_LORESERVE:
        // case SHN_LOPROC:
        // case SHN_BEFORE:
        case SHN_AFTER:
        case SHN_HIPROC:
        case SHN_LOOS:
        case SHN_HIOS:
        case SHN_ABS:
        case SHN_COMMON:
        // case SHN_HIRESERVE:
        case SHN_XINDEX:
            return false;
        default:
            return true;
        }
    }
}