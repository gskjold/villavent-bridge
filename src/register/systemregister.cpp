#include "systemregister.h"

String SystemRegister::getFormattedValue(int address) {
    int value = getValue(address);
    if(value == VAL_INVALID)
        return "";
    switch (address) {
        case REG_SYSTEM_TYPE:
            switch(value) {
                case 0:
                    return "VR400";
                case 1:
                    return "VR700";
                case 2:
                    return "VR700DK";
                case 3:
                    return "VR400DE";
                case 4:
                    return "VTC300";
                case 5:
                    return "VTC700";
                case 12:
                    return "VTR150K";
                case 13:
                    return "VTR200B";
                case 14:
                    return "VSR300";
                case 15:
                    return "VSR500";
                case 16:
                    return "VSR150";
                case 17:
                    return "VTR300";
                case 18:
                    return "VTR500";
                case 19:
                    return "VSR300DE";
                case 20:
                    return "VTC200";
                default:
                    return "unknown";
            }
    }
    return String(value);
}
