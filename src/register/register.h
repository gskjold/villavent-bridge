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
    
    enum Flags
    {
       FLG_VALUE_UPDATED = 0x01,
       FLG_PENDING_WRITE = 0x02
    };


    // Context info for single registers
    class SingleReg
    {
       SingleReg() : m_value(VAL_INVALID), m_pending_set_value(VAL_INVALID),m_perm(0),m_flags(0){}
    private:
      String     m_name;
      int32_t    m_value;
      int32_t    m_pending_set_value;
      uint8_t    m_perm;
      uint8_t    m_flags;    

      friend class Register;
    };

public:
    Register(String name, int start, int length, int interval, bool coil) {
        m_name = name;
        m_start = start;
        m_length = length;
        m_interval = interval;
        m_coil = coil;
        m_single_regs = new SingleReg[length]; 
        m_last_updated = 0;
    }
    String getName();
    int getStart();
    int getLength();
    bool isCoil();
    
    int getValue(int address);
    boolean setValue(int address, int value);

    void addRegister(int address, String name, byte permission=PERM_FULL);
    const String &getRegisterName(int address);
    int getRegisterAddress(String &name);
    bool isReadable(int address);
    bool isWriteable(int address);

    virtual String getFormattedValue(int address);
    virtual boolean setFormattedValue(int address, String &value);

    boolean needsUpdate(unsigned long millis);
    void setLastUpdated(unsigned long millis);

private:
    int addr2Index( int address, const char *source );

private:
    String          m_name;
    int32_t         m_start, m_length, m_interval;
    bool            m_coil;
    SingleReg      *m_single_regs;
    uint32_t        m_last_updated;
    String          m_empty_string;
};

#endif
