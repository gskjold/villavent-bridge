#ifndef _FANREGISTER_H
#define _FANREGISTER_H

#include "register.h"

#define REG_FAN_SPEED_LEVEL 101
#define REG_FAN_SF_FLOW_LOW 102
#define REG_FAN_EF_FLOW_LOW 103
#define REG_FAN_SF_FLOW_NOM 104
#define REG_FAN_EF_FLOW_NOM 105
#define REG_FAN_SF_FLOW_HIGH 106
#define REG_FAN_EF_FLOW_HIGH 107
#define REG_FAN_FLOW_UNITS 108
#define REG_FAN_SF_PWM 109
#define REG_FAN_EF_PWM 110
#define REG_FAN_SF_RPM 111
#define REG_FAN_EF_RPM 112
#define REG_FAN_SPEED_LVL_CD 113
#define REG_FAN_ALLOW_MANUAL_FAN_STOP 114
#define REG_FAN_SPEED_LOG_RESET 115
#define REG_FAN_SPEED_LOG_SF_LVL1 116
#define REG_FAN_SPEED_LOG_SF_LVL2 117
#define REG_FAN_SPEED_LOG_SF_LVL3 118
#define REG_FAN_SPEED_LOG_SF_LVL4 119
#define REG_FAN_SPEED_LOG_SF_LVL5 120
#define REG_FAN_SPEED_LOG_EF_LVL1 121
#define REG_FAN_SPEED_LOG_EF_LVL2 122
#define REG_FAN_SPEED_LOG_EF_LVL3 123
#define REG_FAN_SPEED_LOG_EF_LVL4 124
#define REG_FAN_SPEED_LOG_EF_LVL5 125
#define REG_FAN_SPEED_LOG_SF_NR_LVL1 126
#define REG_FAN_SPEED_LOG_SF_NR_LVL2 127
#define REG_FAN_SPEED_LOG_SF_NR_LVL3 128
#define REG_FAN_SPEED_LOG_SF_NR_LVL4 129
#define REG_FAN_SPEED_LOG_SF_NR_LVL5 130
#define REG_FAN_SPEED_LOG_EF_NR_LVL1 131
#define REG_FAN_SPEED_LOG_EF_NR_LVL2 132
#define REG_FAN_SPEED_LOG_EF_NR_LVL3 133
#define REG_FAN_SPEED_LOG_EF_NR_LVL4 134
#define REG_FAN_SPEED_LOG_EF_NR_LVL5 135
#define REG_FAN_SYSTEM_CURVE_SF 136
#define REG_FAN_SYSTEM_CURVE_EF 137
#define REG_FAN_CONTROL_TYPE 138

class FanRegister : public Register {
    public:
        FanRegister() : Register("fan", 100, 38, 1000, false) {
            addRegister(REG_FAN_SPEED_LEVEL, "fan/speed");
            addRegister(REG_FAN_SF_FLOW_LOW, "fan/supply/flow/low");
            addRegister(REG_FAN_EF_FLOW_LOW, "fan/extract/flow/low");
            addRegister(REG_FAN_SF_FLOW_NOM, "fan/supply/flow/nominal");
            addRegister(REG_FAN_EF_FLOW_NOM, "fan/extract/flow/nominal");
            addRegister(REG_FAN_SF_FLOW_HIGH, "fan/supply/flow/high");
            addRegister(REG_FAN_EF_FLOW_HIGH, "fan/extract/flow/high");
            addRegister(REG_FAN_FLOW_UNITS, "fan/flow/unit");
            addRegister(REG_FAN_SF_PWM, "fan/supply/pwm", PERM_READ_ONLY);
            addRegister(REG_FAN_EF_PWM, "fan/extract/pwm", PERM_READ_ONLY);
            addRegister(REG_FAN_SF_RPM, "fan/supply/rpm", PERM_READ_ONLY, false, 10000 );
            addRegister(REG_FAN_EF_RPM, "fan/extract/rpm", PERM_READ_ONLY, false, 10000 );
            addRegister(REG_FAN_SPEED_LVL_CD, "fan/speed/display", PERM_READ_ONLY);
            addRegister(REG_FAN_ALLOW_MANUAL_FAN_STOP, "fan/allowstop");
            addRegister(REG_FAN_SPEED_LOG_RESET, "fan/speed/log/reset", PERM_WRITE_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_LVL1, "fan/speed/log/supply/level/1", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_LVL2, "fan/speed/log/supply/level/2", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_LVL3, "fan/speed/log/supply/level/3", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_LVL4, "fan/speed/log/supply/level/4", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_LVL5, "fan/speed/log/supply/level/5", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_LVL1, "fan/speed/log/extract/level/1", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_LVL2, "fan/speed/log/extract/level/2", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_LVL3, "fan/speed/log/extract/level/3", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_LVL4, "fan/speed/log/extract/level/4", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_LVL5, "fan/speed/log/extract/level/5", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_NR_LVL1, "fan/speed/log/supply/nr/level/1", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_NR_LVL2, "fan/speed/log/supply/nr/level/2", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_NR_LVL3, "fan/speed/log/supply/nr/level/3", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_NR_LVL4, "fan/speed/log/supply/nr/level/4", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_SF_NR_LVL5, "fan/speed/log/supply/nr/level/5", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_NR_LVL1, "fan/speed/log/extract/nr/level/1", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_NR_LVL2, "fan/speed/log/extract/nr/level/2", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_NR_LVL3, "fan/speed/log/extract/nr/level/3", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_NR_LVL4, "fan/speed/log/extract/nr/level/4", PERM_READ_ONLY);
            addRegister(REG_FAN_SPEED_LOG_EF_NR_LVL5, "fan/speed/log/extract/nr/level/5", PERM_READ_ONLY);
            addRegister(REG_FAN_SYSTEM_CURVE_SF, "fan/supply/systemcurve");
            addRegister(REG_FAN_SYSTEM_CURVE_EF, "fan/extract/systemcurve");
            addRegister(REG_FAN_CONTROL_TYPE, "fan/controltype");
        };

        String getFormattedValue(int address) const;
        boolean setFormattedValue(int address, const String &value, bool setpending=false);
};

#endif
