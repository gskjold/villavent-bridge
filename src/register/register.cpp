#include "register.h"

String Register::getName() {
    return this->name;
}

int Register::getStart() {
    return this->start;
}

int Register::getLength() {
    return this->length;
}

bool Register::isCoil() {
    return this->coil;
}

int Register::getValue(int address) {
    int idx = address-start-1;
    if(idx < 0 || idx >= length) {
        Serial.print("getValue Incorrect index ");
        Serial.print(idx);
        Serial.print(" for address ");
        Serial.println(address);
        return false;
    }
    return this->values[idx];
}

boolean Register::setValue(int address, int value) {
    int idx = address-start-1;
    if(idx < 0 || idx >= length) {
        Serial.print("setValue Incorrect index ");
        Serial.print(idx);
        Serial.print(" for address ");
        Serial.println(address);
        return false;
    }
    int current = getValue(address);
    this->values[idx] = value;
    return current != value || !isReadable(address);
}

void Register::addRegister(int address, String name, byte permission) {
    int idx = address-start-1;
    if(idx < 0 || idx >= length) {
        Serial.print("addRegister Incorrect index ");
        Serial.print(idx);
        Serial.print(" for address ");
        Serial.println(address);
        return;
    }
    this->names[idx] = name;
    this->perms[idx] = permission;
    this->values[idx] = isReadable(address) ? -1 : 0;
}

String* Register::getRegisterName(int address) {
    int idx = address-start-1;
    return &(this->names[idx]);
}

String Register::getFormattedValue(int address) {
    return String(getValue(address));
}

int Register::getRegisterAddress(String &name) {
    for(int i=0; i<length; i++) {
        String registerName = names[i];
        if(registerName == name) {
            return start + i + 1;
        }
    }
    return REG_INVALID;
}

bool Register::isReadable(int address) {
    int idx = address-start-1;
    byte perm = this->perms[idx];
    return perm == PERM_FULL || perm == PERM_READ_ONLY;
}

bool Register::isWriteable(int address) {
    int idx = address-start-1;
    byte perm = this->perms[idx];
    return perm == PERM_FULL || perm == PERM_WRITE_ONLY;
}

boolean Register::setFormattedValue(int address, String &value) {
    return false;
}

boolean Register::needsUpdate(unsigned long millis) {
    return lastUpdated == 0 || (millis - lastUpdated) > interval;
}

void Register::setLastUpdated(unsigned long millis) {
    this->lastUpdated = millis;
}
