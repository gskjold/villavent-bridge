#include "alarmcoil.h"

String AlarmCoil::getFormattedValue(int address) {
    return getValue(address) == 0 ? "OFF" : "ON";
}
