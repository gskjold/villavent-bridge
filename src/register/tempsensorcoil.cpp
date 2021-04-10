#include "tempsensorstatecoil.h"

String TempSensorStateCoil::getFormattedValue(int address) {
    return getValue(address) == 0 ? "OFF" : "ON";
}
