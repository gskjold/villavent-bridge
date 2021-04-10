#ifndef _HEATERREGISTER_H
#define _HEATERREGISTER_H

#include "register.h"

#define REG_HC_HEATER_TYPE 201
#define REG_HC_COOLER_TYPE 202
#define REG_HC_WCWH_STATE 203
#define REG_HC_WC_SIGNAL 204
#define REG_HC_WH_SIGNAL 205
#define REG_HC_FPS_LEVEL 206
#define REG_HC_TEMP_LVL 207
#define REG_HC_TEMP_SP 208
#define REG_HC_TEMP_LVL1 209
#define REG_HC_TEMP_LVL2 210
#define REG_HC_TEMP_LVL3 211
#define REG_HC_TEMP_LVL4 212
#define REG_HC_TEMP_LVL5 213
#define REG_HC_TEMP_IN1 214
#define REG_HC_TEMP_IN2 215
#define REG_HC_TEMP_IN3 216
#define REG_HC_TEMP_IN4 217
#define REG_HC_TEMP_IN5 218
#define REG_HC_TEMP_STATE 219
#define REG_HC_PREHEATER_TYPE 220
#define REG_HC_HEATER_TEMP_SP 221

class HeaterRegister : public Register {
    public:
        HeaterRegister() : Register("hc", 200, 21, 2000, false) {
            addRegister(REG_HC_HEATER_TYPE, "heater/type");
            addRegister(REG_HC_COOLER_TYPE, "cooler/type");
            addRegister(REG_HC_WCWH_STATE, "hc/state", PERM_READ_ONLY);
            addRegister(REG_HC_WC_SIGNAL, "cooler/signal", PERM_READ_ONLY);
            addRegister(REG_HC_WH_SIGNAL, "heater/signal", PERM_READ_ONLY);
            addRegister(REG_HC_FPS_LEVEL, "temperature/frostprotection");
            addRegister(REG_HC_TEMP_LVL, "temperature/level");
            addRegister(REG_HC_TEMP_SP, "temperature/setpoint", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_LVL1, "temperature/level1", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_LVL2, "temperature/level2", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_LVL3, "temperature/level3", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_LVL4, "temperature/level4", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_LVL5, "temperature/level5", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_IN1, "temperature/supply", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_IN2, "temperature/extract", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_IN3, "temperature/exhaust", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_IN4, "temperature/protection", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_IN5, "temperature/intake", PERM_READ_ONLY);
            addRegister(REG_HC_TEMP_STATE, "", PERM_NONE);
            addRegister(REG_HC_PREHEATER_TYPE, "preheater/type");
            addRegister(REG_HC_HEATER_TEMP_SP, "heater/setpoint", PERM_READ_ONLY);
        };

        String getFormattedValue(int address);
        boolean setFormattedValue(int address, String &value);
};

#endif
