#include "filterregister.h"

boolean FilterRegister::setFormattedValue(int address, const String &value, bool setpending) {
    int val;
    switch(address) {
        case REG_FILTER_PER:
            val = value.toInt();
            if(val >= 6 && val <= 12) {
                return setpending ? setNewPendingWriteValue(REG_FILTER_PER, val) : setValue(REG_FILTER_PER, val);
            }
            break;
        case REG_FILTER_DAYS:
            val = value.toInt();
            if(val == 0) {
                return setpending ? setNewPendingWriteValue(REG_FILTER_DAYS, val) : setValue(REG_FILTER_DAYS, val);
            }
            break;
    }
    return false;
}
