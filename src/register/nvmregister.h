#ifndef _NVMREGISTER_H
#define _NVMREGISTER_H

#include "register.h"

#define REG_STORE_NVM 549

class NvmRegister : public Register {
    public:
        NvmRegister() : Register("nvm", 548, 1, 60000, false) {
            addRegister(REG_STORE_NVM, "nvm/store", PERM_WRITE_ONLY);
        };

        boolean setFormattedValue(int address, String &value);
};

#endif
