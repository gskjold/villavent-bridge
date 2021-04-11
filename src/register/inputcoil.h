#ifndef _INPUTCOIL_H
#define _INPUTCOIL_H

#include "register.h"

#define COIL_DI1 11201
#define COIL_DI2 11202
#define COIL_DI3 11203
#define COIL_DI4 11204
#define COIL_DI5 11205
#define COIL_DI6 11206
#define COIL_DI7 11207

class InputCoil : public Register {
    public:
        InputCoil() : Register("inputcoil", 11200, 7, 500, true) {
            addRegister(COIL_DI1, "input/1", PERM_WRITE_ONLY);
            addRegister(COIL_DI2, "input/2", PERM_WRITE_ONLY);
            addRegister(COIL_DI3, "input/3", PERM_WRITE_ONLY);
            addRegister(COIL_DI4, "input/stopheat", PERM_WRITE_ONLY);
            addRegister(COIL_DI5, "input/extended", PERM_WRITE_ONLY);
            addRegister(COIL_DI6, "rotor/sensor", PERM_READ_ONLY);
            addRegister(COIL_DI7, "input/away", PERM_WRITE_ONLY);
        };

        boolean setFormattedValue(int address, String &value);
        String getFormattedValue(int address);
};

#endif
