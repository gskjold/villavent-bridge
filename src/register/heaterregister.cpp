#include "heaterregister.h"

boolean HeaterRegister::setFormattedValue(int address, const String &value, bool setpending) {
    switch(address) {
        case REG_HC_HEATER_TYPE:
        {
            int heater;
            if(value == "0" || value == "none" || value == "no") {
                heater = 0;
            } else if(value == "1" || value == "water") {
                heater = 1;
            } else if(value == "2" || value == "electrical") {
                heater = 2;
            } else if(value == "3" || value == "contactor") {
                heater = 3;
            } else {
                return false;
            }
            return setpending ? setNewPendingWriteValue(REG_HC_HEATER_TYPE, heater) : setValue(REG_HC_HEATER_TYPE, heater);
        }
        case REG_HC_COOLER_TYPE:
        {
            int cooler;
            if(value == "0" || value == "none" || value == "no") {
                cooler = 0;
            } else if(value == "1" || value == "water") {
                cooler = 1;
            } else {
                return false;
            }
            return setpending ? setNewPendingWriteValue(REG_HC_COOLER_TYPE, cooler) : setValue(REG_HC_COOLER_TYPE, cooler);
        }
        case REG_HC_FPS_LEVEL:
        {
            double fps = value.toDouble();
            if(fps >= 7.0 && fps <= 12.0) {
                return setpending ? setNewPendingWriteValue(REG_HC_FPS_LEVEL, ((int) fps) * 10) : setValue(REG_HC_FPS_LEVEL, ((int) fps) * 10);
            } else if(fps >= 70 && fps <= 120) {
                return setpending ? setNewPendingWriteValue(REG_HC_FPS_LEVEL, (int) fps) : setValue(REG_HC_FPS_LEVEL, (int) fps);
            }
            break;
        }
        case REG_HC_TEMP_LVL:
        {
            int lvl = value.toInt();
            if(lvl >= 12 && lvl <= 22) {
                return setpending ? setNewPendingWriteValue(REG_HC_TEMP_LVL, lvl-11) : setValue(REG_HC_TEMP_LVL, lvl-11);
            } else if(lvl < 12 && lvl >= 0) {
                return setpending ? setNewPendingWriteValue(REG_HC_TEMP_LVL, lvl) : setValue(REG_HC_TEMP_LVL, lvl);
            }
            break;
        }
        case REG_HC_PREHEATER_TYPE:
        {
            int preheater = 0;
            if(value == "0" || value == "none" || value == "no") {
                preheater = 0;
            } else if(value == "1" || value == "electrical") {
                preheater = 1;
            } else {
                return false;
            }
            return setpending ? setNewPendingWriteValue(REG_HC_PREHEATER_TYPE, preheater) : setValue(REG_HC_PREHEATER_TYPE, preheater);
        }
        #if SLAVE_MODE
        case REG_HC_TEMP_LVL1:
        case REG_HC_TEMP_LVL2:
        case REG_HC_TEMP_LVL3:
        case REG_HC_TEMP_LVL4:
        case REG_HC_TEMP_LVL5:
        case REG_HC_TEMP_IN1:
        case REG_HC_TEMP_IN2:
        case REG_HC_TEMP_IN3:
        case REG_HC_TEMP_IN4:
        case REG_HC_TEMP_IN5:
            return setValue(address, (int) (value.toDouble()*10));
        #endif
    }
    return false;
}

String HeaterRegister::getFormattedValue(int address) {
    int value = getValue(address);
    if(value == VAL_INVALID)
        return "";
    switch (address) {
        case REG_HC_TEMP_LVL:
            if(value > 0)
                return String(((double) value+11));
            else
                return String(0);
        case REG_HC_FPS_LEVEL:
            return String(((double) value) / 10);
        case REG_HC_TEMP_LVL1:
        case REG_HC_TEMP_LVL2:
        case REG_HC_TEMP_LVL3:
        case REG_HC_TEMP_LVL4:
        case REG_HC_TEMP_LVL5:
        case REG_HC_TEMP_IN1:
        case REG_HC_TEMP_IN2:
        case REG_HC_TEMP_IN3:
        case REG_HC_TEMP_IN4:
        case REG_HC_TEMP_IN5:
                return String(((double) value) / 10, 1);
    }
    return String(value);
}
