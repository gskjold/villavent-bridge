#include "register.h"

String Register::getName() {
    return m_name;
}

int Register::getStart() {
    return m_start;
}

int Register::getLength() {
    return m_length;
}

bool Register::isCoil() {
    return m_coil;
}

int Register::addr2Index( int address, const char *source )
{
  int idx = address-m_start-1;
  if(idx < 0 || idx >= m_length) {
    Serial.print( source );
    Serial.print(" Incorrect index ");
    Serial.print(idx);
    Serial.print(" for address ");
    Serial.println(address);
    return VAL_INVALID;
  }
  return idx;
}


int Register::getValue(int address) {
    int idx = addr2Index( address, "getValue");
    if( idx == VAL_INVALID )
      return VAL_INVALID;
    return m_single_regs[idx].m_value;
}

boolean Register::setValue(int address, int value) {
    int idx = addr2Index( address, "setValue");
    if( idx == VAL_INVALID )
      return false;
    int current = getValue(address);
    m_single_regs[idx].m_value = value;
    return current != value || !isReadable(address);
}

void Register::addRegister(int address, String name, byte permission) {
   int idx = addr2Index( address, "addRegister");
    if( idx == VAL_INVALID )
      return;
    SingleReg &r = m_single_regs[idx];
    r.m_name = name;
    r.m_perm = permission;
    r.m_value = isReadable( address ) ? VAL_INVALID : 0;
    r.m_flags = 0; // TODO add flags?
}

const String &Register::getRegisterName(int address) {
    int idx = addr2Index( address, "getRegisterName");
    if( idx == VAL_INVALID )
    {
      return m_empty_string;
    }
    return (m_single_regs[idx].m_name);
}

String Register::getFormattedValue(int address) {
    int val = getValue(address);
    if(val == VAL_INVALID)
        return "";
    return String(val);
}

int Register::getRegisterAddress(String &name) {
    for(int i=0; i<m_length; i++) {
        if ( m_single_regs[i].m_name == name )
        {
          return m_start + i + 1;
        }
    }
    return REG_INVALID;
}

bool Register::isReadable(int address) {
    int idx = address-m_start-1;
    byte perm = m_single_regs[idx].m_perm;
    return perm == PERM_FULL || perm == PERM_READ_ONLY;
}

bool Register::isWriteable(int address) {
    int idx = address-m_start-1; // TODO Add range check
    byte perm = m_single_regs[idx].m_perm;
    return perm == PERM_FULL || perm == PERM_WRITE_ONLY;
}

boolean Register::setFormattedValue(int address, String &value) {
    #if SLAVE_MODE
        setValue(address, value.toInt());
    #endif
    return false;
}

boolean Register::needsUpdate(unsigned long millis) {
    return m_last_updated == 0 || (millis - m_last_updated) > m_interval;
}

void Register::setLastUpdated(unsigned long millis) {
    m_last_updated = millis;
}
