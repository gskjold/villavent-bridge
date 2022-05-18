#ifndef _CLOCKREGISTER_H
#define _CLOCKREGISTER_H

#include "register.h"

#define REG_CLK_S 551
#define REG_CLK_M 552
#define REG_CLK_H 553
#define REG_CLK_D 554
#define REG_CLK_MNTH 555
#define REG_CLK_Y 556
#define REG_CLK_WD 557

class ClockRegister : public Register {
    public:
        ClockRegister() : Register("clock", 550, 7, 1000, false) {
            addRegister(REG_CLK_S, "clock/second");
            addRegister(REG_CLK_M, "clock/minute");
            addRegister(REG_CLK_H, "clock/hour");
            addRegister(REG_CLK_D, "clock/day");
            addRegister(REG_CLK_MNTH, "clock/month");
            addRegister(REG_CLK_Y, "clock/year");
            addRegister(REG_CLK_WD, "clock/weekday", PERM_READ_ONLY);
        };

        String getFormattedValue(int address);
        boolean setFormattedValue(int address, const String &value, bool setpending=false);
};

#endif
