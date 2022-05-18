#include "nvmregister.h"

boolean NvmRegister::setFormattedValue(int address, const String &value, bool setpending) {
    switch(address) {
        case REG_STORE_NVM:
            String lowvalue(value); lowvalue.toLowerCase();
            if(lowvalue == "1" || lowvalue == "true" || lowvalue == "165") {
                return setpending ? setNewPendingWriteValue(REG_STORE_NVM, 165) : setValue(REG_STORE_NVM, 165);
            }
            break;
    }
    return false;
}
