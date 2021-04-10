#include "rotorregister.h"

String RotorRegister::getFormattedValue(int address) {
    int val = getValue(address);
    switch(address) {
        case REG_ROTOR_RELAY_ACTIVE:
            return val == 0 ? "OFF" : "ON";
    }
    return String(val);
}
