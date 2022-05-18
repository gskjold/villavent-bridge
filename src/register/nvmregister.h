#ifndef _NVMREGISTER_H
#define _NVMREGISTER_H

#include "register.h"

#define REG_STORE_NVM 549

class NvmRegister : public Register {
    public:
        NvmRegister() : Register("nvm", 548, 1, 3600000, false) {
            addRegister(REG_STORE_NVM, "nvm/store", PERM_WRITE_ONLY);
        };

        boolean setFormattedValue(int address, const String &value, bool setpending=false);
};

#endif
