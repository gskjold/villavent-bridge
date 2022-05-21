#include "rotorregister.h"

String RotorRegister::getFormattedValue(int address) const {
    int val = getValue(address);
    if(val == VAL_INVALID)
        return "";
    switch(address) {
        case REG_ROTOR_RELAY_ACTIVE:
            return val == 0 ? "OFF" : "ON";
    }
    return String(val);
}
