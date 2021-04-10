#include "nvmregister.h"

boolean NvmRegister::setFormattedValue(int address, String &value) {
    switch(address) {
        case REG_STORE_NVM:
            value.toLowerCase();
            if(value == "1" || value == "true" || value == "165") {
                return setValue(REG_STORE_NVM, 165);
            }
            break;
    }
    return false;
}
