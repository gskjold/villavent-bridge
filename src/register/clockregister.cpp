#include "clockregister.h"

boolean ClockRegister::setFormattedValue(int address, String &value) {
    int val;
    switch(address) {
        case REG_CLK_S:
        case REG_CLK_M:
            val = value.toInt();
            if(val >= 0 && val < 60) {
                return setValue(address, val);
            }
            break;
        case REG_CLK_H:
            val = value.toInt();
            if(val >= 0 && val < 24) {
                return setValue(REG_CLK_H, val);
            }
            break;
        case REG_CLK_D:
            val = value.toInt();
            if(val > 0 && val <= 31) {
                return setValue(REG_CLK_D, val);
            }
            break;
        case REG_CLK_MNTH:
            val = value.toInt();
            if(val > 0 && val <= 12) {
                return setValue(REG_CLK_MNTH, val);
            }
            break;
        case REG_CLK_Y:
            val = value.toInt();
            if(val >= 2000) {
                val = val - 2000;
                return setValue(REG_CLK_Y, val);
            } else if(val > 0) {
                return setValue(REG_CLK_Y, val);
            }
            break;
    }
    return false;
}

String ClockRegister::getFormattedValue(int address) {
    int value = getValue(address);
    if(value == VAL_INVALID)
        return "";
    switch (address) {
        case REG_CLK_Y:
            value = 2000 + value;
            break;
    }
    return String(value);
}
