#ifndef _SYSTEMERGISTER_H
#define _SYSTEMERGISTER_H

#include "register.h"

#define REG_SYSTEM_TYPE 501
#define REG_SYSTEM_PROG_V_HIGH 502
#define REG_SYSTEM_PROG_V_MID 503
#define REG_SYSTEM_PROG_V_LOW 504
#define REG_SYSTEM_BOOT_PROG_V_HIGH 505
#define REG_SYSTEM_BOOT_PROG_V_MID 506
#define REG_SYSTEM_BOOT_PROG_V_LOW 507

class SystemRegister : public Register {
    public:
        SystemRegister() : Register("system", 500, 7, 3600000, false) {
            addRegister(REG_SYSTEM_TYPE, "system/type", PERM_READ_ONLY);
            addRegister(REG_SYSTEM_PROG_V_HIGH, "system/program/version/high", PERM_READ_ONLY);
            addRegister(REG_SYSTEM_PROG_V_MID, "system/program/version/mid", PERM_READ_ONLY);
            addRegister(REG_SYSTEM_PROG_V_LOW, "system/program/version/low", PERM_READ_ONLY);
            addRegister(REG_SYSTEM_BOOT_PROG_V_HIGH, "system/boot/version/high", PERM_READ_ONLY);
            addRegister(REG_SYSTEM_BOOT_PROG_V_MID, "system/boot/version/mid", PERM_READ_ONLY);
            addRegister(REG_SYSTEM_BOOT_PROG_V_LOW, "system/boot/version/low", PERM_READ_ONLY);
        };

        String getFormattedValue(int address) const;
};

#endif
