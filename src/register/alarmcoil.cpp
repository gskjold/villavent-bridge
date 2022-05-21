#include "alarmcoil.h"

String AlarmCoil::getFormattedValue(int address) const {
    int value = getValue(address);
    if(value == VAL_INVALID)
        return "";
    return value == 0 ? "OFF" : "ON";
}
