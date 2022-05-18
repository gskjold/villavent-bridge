#include "inputcoil.h"

boolean InputCoil::setFormattedValue(int address, const String &value, bool setpending) {
    String val_lower = value; val_lower.toLowerCase();
    switch(address) {
        case COIL_DI1:
        case COIL_DI2:
        case COIL_DI3:
        case COIL_DI4:
        case COIL_DI5:
        case COIL_DI7:
            if(val_lower == "0" || val_lower == "off" || val_lower == "false") {
                return setpending ? setNewPendingWriteValue(address, 0) :  setValue(address, 0);
            } else if(val_lower == "1" || val_lower == "on" || val_lower == "true") {
                return setpending ? setNewPendingWriteValue(address, 1) : setValue(address, 1);
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
