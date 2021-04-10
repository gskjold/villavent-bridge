#ifndef _ROTORREGISTER_H
#define _ROTORREGISTER_H

#include "register.h"

#define REG_ROTOR_STATE 351
#define REG_ROTOR_RELAY_ACTIVE 352

class RotorRegister : public Register {
    public:
        RotorRegister() : Register("rotor", 350, 2, 200, false) {
            addRegister(REG_ROTOR_STATE, "rotor/state", PERM_READ_ONLY);
            addRegister(REG_ROTOR_RELAY_ACTIVE, "rotor/relay", PERM_READ_ONLY);
        };

        String getFormattedValue(int address);
};

#endif
