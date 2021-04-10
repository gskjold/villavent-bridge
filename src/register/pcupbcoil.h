#ifndef _PCUPBCOIL_H
#define _PCUPBCOIL_H

#include "register.h"

#define COIL_HC_PREHEATER_RELAY 12001
#define COIL_HC_HEATER_RELAY 12002
#define COIL_HC_COMMON_RELAY 12003

class PcuPbCoil : public Register {
    public:
        PcuPbCoil() : Register("pcupb", 12000, 3, 200, true) {
            addRegister(COIL_HC_PREHEATER_RELAY, "preheater/relay", PERM_READ_ONLY);
            addRegister(COIL_HC_HEATER_RELAY, "heater/relay", PERM_READ_ONLY);
            addRegister(COIL_HC_COMMON_RELAY, "hc/relay", PERM_READ_ONLY);
        };

        String getFormattedValue(int address);
};

#endif
