#ifndef _REGISTER_H
#define _REGISTER_H

#include "Arduino.h"

#define REG_INVALID -1
#define VAL_INVALID -404

#define PERM_NONE -1
#define PERM_FULL 0
#define PERM_READ_ONLY 1
#define PERM_WRITE_ONLY 2

/* *************************************************************
*  Register block, keeping a list of single registers for 
*  the block.
*
*
*/
class Register 
{
    
    enum Flags
    {
       FLG_VALUE_UPDATED = 0x01,
       FLG_PENDING_WRITE = 0x02,
    };

    // ******************************************    
    // Context info kept for single registers
    class SingleReg
    {
       SingleReg() : m_value(VAL_INVALID), m_pending_set_value(VAL_INVALID),m_perm(0),m_flags(0){}
    private:
      String     m_name;
      int32_t    m_value;
      int32_t    m_pending_set_value;
      uint8_t    m_perm;
      uint8_t    m_flags;    

      friend class Register; // to let Register access private members
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
    String getName() const ;
    int getStart() const ;
    int getLength() const;
    bool isCoil() const;

    // Use this method to set a new value to be written
    // by the registry manager.
    // returns true if there was a change.
    bool    setNewPendingWriteValue( int32_t address, int32_t value );
    // check if there is a pending write request 
    bool    hasPendingWrite(int32_t address) const;
    // Get value of pending write.
    int32_t getPendingWriteValue( int32_t address ) const;
    // Clear pending write flag
    void confirmPendingWrite( int32_t address );



    bool hasUpdatedValue(int32_t address) const;  // check update flag.
    void confirmUpdate(int32_t address);    // acknowledge the updated value.
    int getValue(int address) const;              // get latest read value.


    void addRegister(int address, const String &name, byte permission=PERM_FULL);
    const String &getRegisterName(int address) const;
    int getRegisterAddress(const String &name) const;
    bool isReadable(int address) const;
    bool isWriteable(int address) const;

    virtual String getFormattedValue(int address) const;
    virtual boolean setFormattedValue(int address, const String &value, bool setpending=false);

    boolean  needsUpdate(uint32_t millis) const;
    void     setLastUpdated(uint32_t millis);
    uint32_t getLastUpdated() const { return m_last_updated; }
    int32_t  getInterval() const { return m_interval; }    

protected:
    boolean setValue(int address, int value); // set value, return true if changed.

    int addr2Index( int address, const char *source ) const;

private:
    String          m_name;
    int32_t         m_start, m_length, m_interval;
    bool            m_coil;
    SingleReg      *m_single_regs;
    uint32_t        m_last_updated;
    String          m_empty_string;
    //uint32_t        m_illegal_address_count;


    friend class RegisterManager; // grant private access to the manager
};

#endif
