#ifndef _REGISTER_H
#define _REGISTER_H

#include "Arduino.h"

#define REG_INVALID -1
#define VAL_INVALID -404

#define PERM_NONE -1
#define PERM_FULL 0
#define PERM_READ_ONLY 1
#define PERM_WRITE_ONLY 2

class Register {
public:
    Register(String name, int start, int length, int interval, bool coil) {
        this->name = name;
        this->start = start;
        this->length = length;
        this->interval = interval;
        this->coil = coil;
        this->values = new int[length];
        this->names = new String[length];
        this->perms = new byte[length];
        for(int i=0;i<length;i++) {
            this->values[i] = VAL_INVALID;
            this->perms[i] = 0;
        }
    }
    String getName();
    int getStart();
    int getLength();
    bool isCoil();
    
    int getValue(int address);
    boolean setValue(int address, int value);

    void addRegister(int address, String name, byte permission=PERM_FULL);
    String* getRegisterName(int address);
    int getRegisterAddress(String &name);
    bool isReadable(int address);
    bool isWriteable(int address);

    virtual String getFormattedValue(int address);
    virtual boolean setFormattedValue(int address, String &value);

    boolean needsUpdate(unsigned long millis);
    void setLastUpdated(unsigned long millis);

private:
    String name;
    int start, length, interval;
    bool coil;
    int *values;
    String *names;
    byte *perms;
    unsigned long lastUpdated = 0;
};

#endif
