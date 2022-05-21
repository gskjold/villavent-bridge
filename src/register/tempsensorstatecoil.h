#ifndef _TEMPSENSORSTATECOIL_H
#define _TEMPSENSORSTATECOIL_H

#include "register.h"

#define COIL_HC_TEMP_IN1_STATE 3489
#define COIL_HC_TEMP_IN2_STATE 3490
#define COIL_HC_TEMP_IN3_STATE 3491
#define COIL_HC_TEMP_IN4_STATE 3492
#define COIL_HC_TEMP_IN5_STATE 3493

class TempSensorStateCoil : public Register {
    public:
        TempSensorStateCoil() : Register("sensor", 3488, 5, 10000, true) {
            addRegister(COIL_HC_TEMP_IN1_STATE, "temperature/supply/alarm", PERM_READ_ONLY);
            addRegister(COIL_HC_TEMP_IN2_STATE, "temperature/extract/alarm", PERM_READ_ONLY);
            addRegister(COIL_HC_TEMP_IN3_STATE, "temperature/exhaust/alarm", PERM_READ_ONLY);
            addRegister(COIL_HC_TEMP_IN4_STATE, "temperature/protection/alarm", PERM_READ_ONLY);
            addRegister(COIL_HC_TEMP_IN5_STATE, "temperature/intake/alarm", PERM_READ_ONLY);
        };

        String getFormattedValue(int address) const;
};

#endif
