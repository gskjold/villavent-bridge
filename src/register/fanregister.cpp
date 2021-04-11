#include "fanregister.h"

boolean FanRegister::setFormattedValue(int address, String &value) {
    switch(address) {
        // TODO: limitations
        case REG_FAN_SPEED_LEVEL:
        case REG_FAN_SF_FLOW_LOW:
        case REG_FAN_EF_FLOW_LOW:
        case REG_FAN_SF_FLOW_NOM:
        case REG_FAN_EF_FLOW_NOM:
        case REG_FAN_SF_FLOW_HIGH:
        case REG_FAN_EF_FLOW_HIGH:
        case REG_FAN_SYSTEM_CURVE_SF:
        case REG_FAN_SYSTEM_CURVE_EF:
            return setValue(address, value.toInt());
        case REG_FAN_FLOW_UNITS:
        {
            value.toLowerCase();
            if(value == "0" || value == "liter_per_second" || value == "liter" || value == "l" || value == "lps") {
                return setValue(REG_FAN_FLOW_UNITS, 0);
            } else if(value == "1" || value == "m3_per_hour" || value == "m3" || value == "m3ph") {
                return setValue(REG_FAN_FLOW_UNITS, 1);
            } else {
                return false;
            }
        }
        case REG_FAN_ALLOW_MANUAL_FAN_STOP:
        {
            value.toLowerCase();
            if(value == "0" || value == "false") {
                return setValue(REG_FAN_ALLOW_MANUAL_FAN_STOP, 0);
            } else if(value == "1" || value == "true") {
                return setValue(REG_FAN_ALLOW_MANUAL_FAN_STOP, 1);
            } else {
                return false;
            }
        }
        case REG_FAN_SPEED_LOG_RESET:
            if(value == "1" || value == "true" || value == "90") {
                return setValue(REG_FAN_SPEED_LOG_RESET, 90);
            }
            break;
        case REG_FAN_CONTROL_TYPE:
        {
            value.toLowerCase();
            if(value == "0" || value == "airflow" || value == "air_flow") {
                return setValue(REG_FAN_CONTROL_TYPE, 0);
            } else if(value == "1" || value == "speed") {
                return setValue(REG_FAN_CONTROL_TYPE, 1);
            } else {
                return false;
            }
        }
        #if SLAVE_MODE
        case REG_FAN_SF_PWM:
        case REG_FAN_EF_PWM:
            return setValue(address, (int) (value.toDouble()*10));
        default:
            return setValue(address, value.toInt());
        #endif
    }
    return false;
};

String FanRegister::getFormattedValue(int address) {
    int value = getValue(address);
    if(value == VAL_INVALID)
        return "";
    switch (address) {
        case REG_FAN_SF_PWM:
        case REG_FAN_EF_PWM:
            return String(((double) value) / 10, 2);
        case REG_FAN_ALLOW_MANUAL_FAN_STOP:
            return value == 0 ? "OFF" : "ON";
    }
    return String(value);
}
