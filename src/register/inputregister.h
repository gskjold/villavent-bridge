#ifndef _INPUTREGISTER_H
#define _INPUTREGISTER_H

#include "register.h"

#define REG_DI_ALL 701
#define REG_DI_EXT_RUNNING_M 702
#define REG_DI_EXT_RUNNING_SPEED_LVL 703
#define REG_DI1_SF_LVL 704
#define REG_DI1_EF_LVL 705
#define REG_DI2_SF_LVL 706
#define REG_DI2_EF_LVL 707
#define REG_DI3_SF_LVL 708
#define REG_DI3_EF_LVL 709

class InputRegister : public Register {
    public:
        InputRegister() : Register("inputregister", 700, 9, 10000, false) {
            addRegister(REG_DI_ALL, "input/all", PERM_READ_ONLY);
            addRegister(REG_DI_EXT_RUNNING_M, "input/extended/minutes");
            addRegister(REG_DI_EXT_RUNNING_SPEED_LVL, "input/extended/speed");
            addRegister(REG_DI1_SF_LVL, "input/1/sf");
            addRegister(REG_DI1_EF_LVL, "input/1/ef");
            addRegister(REG_DI2_SF_LVL, "input/2/sf");
            addRegister(REG_DI2_EF_LVL, "input/2/ef");
            addRegister(REG_DI3_SF_LVL, "input/3/sf");
            addRegister(REG_DI3_EF_LVL, "input/3/ef");
        };

        boolean setFormattedValue(int address, String &value);
};

#endif
