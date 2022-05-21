#include "pcupbcoil.h"

String PcuPbCoil::getFormattedValue(int address) const {
    int value = getValue(address);
    if(value == VAL_INVALID)
        return "";
    if(value == 1) {
        return "ON";
    } else if(value == 0) {
        return "OFF";
    }
    return String(value);
}
