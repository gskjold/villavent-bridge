#include "inputregister.h"

boolean InputRegister::setFormattedValue(int address, String &value) {
    int val;
    switch(address) {
        case REG_DI_EXT_RUNNING_M:
            val = value.toInt();
            if(val >= 0) {
                return setValue(REG_DI_EXT_RUNNING_M, val);
            }
            break;
        case REG_DI_EXT_RUNNING_SPEED_LVL:
        case REG_DI1_SF_LVL:
        case REG_DI1_EF_LVL:
        case REG_DI2_SF_LVL:
        case REG_DI2_EF_LVL:
        case REG_DI3_SF_LVL:
        case REG_DI3_EF_LVL:
            val = value.toInt();
            if(val >= 0 && val <= 3) {
                return setValue(address, val);
            }
            break;
    }
    return false;
}
