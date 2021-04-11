#include "inputcoil.h"

boolean InputCoil::setFormattedValue(int address, String &value) {
    switch(address) {
        case COIL_DI1:
        case COIL_DI2:
        case COIL_DI3:
        case COIL_DI4:
        case COIL_DI5:
        case COIL_DI7:
            value.toLowerCase();
            if(value == "0" || value == "off" || value == "false") {
                return setValue(address, 0);
            } else if(value == "1" || value == "on" || value == "true") {
                return setValue(address, 1);
            } else {
                return false;
            }
            break;
    }
    return false;
}

String InputCoil::getFormattedValue(int address) {
    int value = getValue(address);
    if(value == VAL_INVALID)
        return "";
    return value == 0 ? "OFF" : "ON";
}
