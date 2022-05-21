#include "fanregister.h"

#include <debugger.h>

boolean FanRegister::setFormattedValue(int address, const String &value, bool setpending) {
    bool ret;
    String vallower( value ); vallower.toLowerCase();
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
            ret = setpending ? setNewPendingWriteValue(address, value.toInt() ) :  setValue(address, value.toInt());
            debugD("Set FAN addr %d integer %d,  %s", address, value.toInt(), ret ? "true":"false");
            return ret;
        case REG_FAN_FLOW_UNITS:
        {
            
            if(vallower == "0" || vallower == "liter_per_second" || vallower == "liter" || vallower == "l" || vallower == "lps") {
                return setpending ? setNewPendingWriteValue(REG_FAN_FLOW_UNITS, 0 ) : setValue(REG_FAN_FLOW_UNITS, 0);
            } else if(vallower == "1" || vallower == "m3_per_hour" || vallower == "m3" || vallower == "m3ph") {
                return setpending ? setNewPendingWriteValue(REG_FAN_FLOW_UNITS, 1 ) : setValue(REG_FAN_FLOW_UNITS, 1);
            } else {
                return false;
            }
        }
        case REG_FAN_ALLOW_MANUAL_FAN_STOP:
        {
            if(vallower == "0" || vallower == "false") {
                return setpending ? setNewPendingWriteValue(REG_FAN_ALLOW_MANUAL_FAN_STOP, 0 ) : setValue(REG_FAN_ALLOW_MANUAL_FAN_STOP, 0);
            } else if(vallower == "1" || vallower == "true") {
                return setpending ? setNewPendingWriteValue(REG_FAN_ALLOW_MANUAL_FAN_STOP, 1 ) : setValue(REG_FAN_ALLOW_MANUAL_FAN_STOP, 1);
            } else {
                return false;
            }
        }
        case REG_FAN_SPEED_LOG_RESET:
            if(value == "1" || value == "true" || value == "90") {
                return setpending ? setNewPendingWriteValue(REG_FAN_SPEED_LOG_RESET, 90) : setValue(REG_FAN_SPEED_LOG_RESET, 90);
            }
            break;
        case REG_FAN_CONTROL_TYPE:
        {
            if(vallower == "0" || vallower == "airflow" || vallower == "air_flow") {
                return setpending ? setNewPendingWriteValue(REG_FAN_CONTROL_TYPE, 0) : setValue(REG_FAN_CONTROL_TYPE, 0);
            } else if(vallower == "1" || vallower == "speed") {
                return setpending ? setNewPendingWriteValue(REG_FAN_CONTROL_TYPE, 1) : setValue(REG_FAN_CONTROL_TYPE, 1);
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

String FanRegister::getFormattedValue(int address) const {
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
