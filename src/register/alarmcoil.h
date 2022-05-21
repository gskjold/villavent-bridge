#ifndef _ALARMCOIL_H
#define _ALARMCOIL_H

#include "register.h"

#define COIL_ALARMS_FILTER 12801
#define COIL_ALARMS_FAN 12802
#define COIL_ALARMS_NOT_USED 12803
#define COIL_ALARMS_ROTOR 12804
#define COIL_ALARMS_FROST 12805
#define COIL_ALARMS_PCU_PB 12806
#define COIL_ALARMS_TEMPERATURE_SENSOR 12807
#define COIL_ALARMS_EMERGENCY_THERMOSTAT 12808
#define COIL_ALARMS_DAMPER 12809

class AlarmCoil : public Register {
    public:
        AlarmCoil() : Register("alarmcoil", 12800, 9, 10000, true) {
            addRegister(COIL_ALARMS_FILTER, "filter/alarm", PERM_READ_ONLY);
            addRegister(COIL_ALARMS_FAN, "fan/alarm", PERM_READ_ONLY);
            addRegister(COIL_ALARMS_NOT_USED, "", PERM_NONE);
            addRegister(COIL_ALARMS_ROTOR, "rotor/alarm", PERM_READ_ONLY);
            addRegister(COIL_ALARMS_FROST, "frost/alarm", PERM_READ_ONLY);
            addRegister(COIL_ALARMS_PCU_PB, "pcupb/alarm", PERM_READ_ONLY);
            addRegister(COIL_ALARMS_TEMPERATURE_SENSOR, "temperature/alarm", PERM_READ_ONLY);
            addRegister(COIL_ALARMS_EMERGENCY_THERMOSTAT, "thermostat/alarm", PERM_READ_ONLY);
            addRegister(COIL_ALARMS_DAMPER, "damper/alarm", PERM_READ_ONLY);
        };

        String getFormattedValue(int address) const;
};

#endif
