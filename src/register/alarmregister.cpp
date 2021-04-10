#include "alarmregister.h"

String AlarmRegister::getFormattedValue(int address) {
    return getValue(address) == 0 ? "OFF" : "ON";
}
