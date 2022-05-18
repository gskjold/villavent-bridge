#include "register.h"
#include "../debugger.h"

/* ******************************************************** */
String Register::getName() const {
    return m_name;
}

int Register::getStart() const {
    return m_start;
}

int Register::getLength() const {
    return m_length;
}

bool Register::isCoil() const {
    return m_coil;
}

/* ******************************************************** */
int Register::addr2Index( int address, const char *source )  const
{
  int idx = address-m_start-1;
  if(idx < 0 || idx >= m_length) {
    debugE("%s incorrect index %d for address %d", idx, address );
    return VAL_INVALID;
  }
  return idx;
}


/* ******************************************************** */
int Register::getValue(int address) const {
    int idx = addr2Index( address, "getValue");
    if( idx == VAL_INVALID )
      return VAL_INVALID;
    return m_single_regs[idx].m_value;
}

/* ******************************************************** */
// be aware that this method must not be called without holding
// the register manager semaphore.
boolean Register::setValue(int address, int value) 
{
    int idx = addr2Index( address, "setValue");
    if( idx == VAL_INVALID )
      return false;

    SingleReg &reg = m_single_regs[idx];
    
    int current = reg.m_value; // get current value.

    reg.m_value = value;       // set new value.

    bool changed =  (current != value) || !isReadable(address);
    if ( changed )
      reg.m_flags |= FLG_VALUE_UPDATED;

    return changed;
}

/* ******************************************************** */
bool  Register::hasUpdatedValue(int32_t address) const
{
    int32_t idx = addr2Index( address, "hasUpdatedValue" );
    if( idx == VAL_INVALID )
      return false;

    return (m_single_regs[idx].m_flags & FLG_VALUE_UPDATED);  
}

/* ******************************************************** */
void Register::confirmUpdate( int32_t address )
{
    int idx = addr2Index( address, "confirmUpdate");
    if( idx == VAL_INVALID )
       return ;
    
    m_single_regs[idx].m_flags &= (~FLG_VALUE_UPDATED);
}


/* ******************************************************** */
bool  Register::hasPendingWrite(int32_t address) const
{
    int32_t idx = addr2Index( address, "hasPendingWrite" );
    if( idx == VAL_INVALID )
      return false;

    return m_single_regs[idx].m_flags & FLG_PENDING_WRITE;  
}

/* ******************************************************** */
int32_t Register::getPendingWriteValue( int32_t address ) const
{
    int32_t idx = addr2Index( address, "getPendingWriteValue" );
    if( idx == VAL_INVALID )
      return false;

    return m_single_regs[idx].m_pending_set_value;  
}

// 
/* ******************************************************** */
bool   Register::setNewPendingWriteValue( int32_t address, int32_t value )
{
    int idx = addr2Index( address, "setNewPendingWriteValue");
    if( idx == VAL_INVALID )
     return false;

    SingleReg &reg = m_single_regs[idx];

    int current = reg.m_pending_set_value; // get current value.

    reg.m_pending_set_value = value;       // set new value.

    bool changed =  (current != value);
    if ( changed )
      reg.m_flags |= FLG_PENDING_WRITE;

    return changed;
        
}

/* ******************************************************** */
void Register::confirmPendingWrite( int32_t address )
{
    int idx = addr2Index( address, "confirmPendingWrite");
    if( idx == VAL_INVALID )
       return ;
    
    m_single_regs[idx].m_flags &= ~FLG_PENDING_WRITE;
}


/* ******************************************************** */
void Register::addRegister(int address, const String &name, byte permission) {
   int idx = addr2Index( address, "addRegister");
    if( idx == VAL_INVALID )
      return;
    SingleReg &r = m_single_regs[idx];
    r.m_name = name;
    r.m_perm = permission;
    r.m_value = isReadable( address ) ? VAL_INVALID : 0;
    r.m_flags = 0; // TODO add flags?
}

/* ******************************************************** */
const String &Register::getRegisterName(int address) const {
    int idx = addr2Index( address, "getRegisterName");
    if( idx == VAL_INVALID )
      return m_empty_string;

    return (m_single_regs[idx].m_name);
}

/* ******************************************************** */
String Register::getFormattedValue(int address) const {
    int val = getValue(address);
    if(val == VAL_INVALID)
        return "";
    return String(val);
}

/* ******************************************************** */
int Register::getRegisterAddress(const String &name) const 
{
    for(int i=0; i<m_length; i++) {
        if ( m_single_regs[i].m_name == name )
        {
          return m_start + i + 1;
        }
    }
    return REG_INVALID;
}

/* ******************************************************** */
bool Register::isReadable(int address) const {
    int idx = addr2Index( address, "isReadable");
    if( idx == VAL_INVALID )
      return false;

    uint8_t perm = m_single_regs[idx].m_perm;

    return perm == PERM_FULL || perm == PERM_READ_ONLY;
}

/* ******************************************************** */
bool Register::isWriteable(int address) const {
    int idx = addr2Index( address, "isWriteable");
    if( idx == VAL_INVALID )
      return false;

    uint8_t perm = m_single_regs[idx].m_perm;
    return perm == PERM_FULL || perm == PERM_WRITE_ONLY;
}

/* ******************************************************** */
boolean Register::setFormattedValue(int address, const String &value, bool setpending) {
    #if SLAVE_MODE
        setValue(address, value.toInt());
    #endif
    return false;
}

/* ******************************************************** */
boolean Register::needsUpdate(uint32_t millis) const {
    return m_last_updated == 0 || (millis - m_last_updated) > m_interval;
}

/* ******************************************************** */
void Register::setLastUpdated(uint32_t millis) {
    m_last_updated = millis;
}
