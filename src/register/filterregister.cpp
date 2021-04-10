#include "filterregister.h"

boolean FilterRegister::setFormattedValue(int address, String &value) {
    int val;
    switch(address) {
        case REG_FILTER_PER:
            val = value.toInt();
            if(val >= 6 && val <= 12) {
                return setValue(REG_FILTER_PER, val);
            }
            break;
        case REG_FILTER_DAYS:
            val = value.toInt();
            if(val == 0) {
                return setValue(REG_FILTER_DAYS, val);
            }
            break;
    }
    return false;
}
