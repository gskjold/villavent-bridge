#ifndef _ALARMREGISTER_H
#define _ALARMREGISTER_H

#include "register.h"

#define REG_ALARMS_ALL 801
#define REG_ALARMS_RELAY_ACTIVE 802

class AlarmRegister : public Register {
    public:
        AlarmRegister() : Register("alarmregister", 800, 2, 10000, false) {
            addRegister(REG_ALARMS_ALL, "alarm", PERM_READ_ONLY);
            addRegister(REG_ALARMS_RELAY_ACTIVE, "alarm/relay", PERM_READ_ONLY);
        };

        String getFormattedValue(int address) const;
};

#endif
