#include "registermanager.h"


#include "register/fanregister.h"
#include "register/heaterregister.h"
#include "register/tempsensorstatecoil.h"
#include "register/rotorregister.h"
#include "register/systemregister.h"
#include "register/nvmregister.h"
#include "register/clockregister.h"
#include "register/filterregister.h"
#include "register/inputregister.h"
#include "register/inputcoil.h"
#include "register/pcupbcoil.h"
#include "register/alarmcoil.h"
#include "register/alarmregister.h"



#include "debugger.h"

/* *************************************************** */
RegisterManager::RegisterManager(ModbusMaster &modbus)
 : OSAPI_Task("regman", STACKSIZE, DEFAULT_PRI,0), // pin to 0, since main seems to be on 1
   m_sem("regman", 1),
   m_enabled(false),
   m_state( STATE_CREATED),
   m_registers(std::vector<Register*>() ),
   m_modbus( modbus ),
   m_reportLastMs(0)
{
  clearStats(); 
}


/* *************************************************** */
RegisterManager::~RegisterManager()
{
  
}

/* *************************************************** */
void RegisterManager::clearStats()
{
  m_lastSuccessfulRead = m_lastFailedRead  = m_numSuccessfulReads = 
  m_numSuccessfulWrites = m_numFailedReads = m_numFailedWrites = 
  m_numReadRetries = m_numWriteRetries = 0;

}
/* *************************************************** */
//void initPrePostTransmit( void (*preTransmit)() , void (*postTransmit)() )
//{
//  m_modbus.preTransmission( preTransmit );
//  m_modbus.postTransmit( postTransmit )
//}


/* *************************************************** */
void RegisterManager::addRegisters()
{
  m_registers.reserve(13); // make it one perfect malloc.

  m_registers.push_back(new FanRegister());
  m_registers.push_back(new HeaterRegister());
  m_registers.push_back(new TempSensorStateCoil());
  m_registers.push_back(new RotorRegister());
  m_registers.push_back(new SystemRegister());
  m_registers.push_back(new NvmRegister());
  m_registers.push_back(new ClockRegister());
  m_registers.push_back(new FilterRegister());
  m_registers.push_back(new InputRegister());
  m_registers.push_back(new InputCoil());
  m_registers.push_back(new PcuPbCoil());
  m_registers.push_back(new AlarmCoil());
  m_registers.push_back(new AlarmRegister());

  debugI("Added %d register blocks", m_registers.size() );

}

/* *************************************************** */
void RegisterManager::enable()
{
  protect( portMAX_DELAY );

  m_enabled = true;

  unprotect();

}

/* *************************************************** */
void RegisterManager::disable()
{
  protect( portMAX_DELAY );

  m_enabled = false;

  unprotect();
}


/* *************************************************** */
int32_t RegisterManager::protect( int32_t mstimeout )
{
  return m_sem.take( mstimeout );
}

/* *************************************************** */
void RegisterManager::unprotect()
{
  m_sem.give();
}


/* *************************************************** */
/* The main thread loop. 
*/
void RegisterManager::taskCode()
{
  uint32_t round_cnt = 0;

  while ( m_do_end==false )
  {
    round_cnt++;

    // Try to grab the semaphore.
    int32_t res = protect( portMAX_DELAY );
    if ( res<0 )
    {
      Sleep( 1000 ); // couldn't get the semaphore, should never happen.
      continue;
    }

    int32_t backoff_time_ms = 10;

    if ( m_enabled==false )
    {
      switchState( STATE_DISABLED );
      debugI("Register manager is disabled");
      backoff_time_ms = 3000;
    }
    else // Enabled state
    {
      // First priority is to update pending writes.
      bool ok = processPendingCommits();

      if ( ok ) // if writes went OK, do reads.
        ok = updateRegisters();
      if ( ok )
      {

        switchState( STATE_CONNECTED );
        backoff_time_ms = 200;
      }
      else
      {
        switchState( STATE_CONNECTING );
        backoff_time_ms = 1000;
      }
    }

    // Let go of the semaphore
    unprotect();

    Sleep( backoff_time_ms );
  }

  m_ended = true;
}

/* *************************************************** */
void RegisterManager::switchState( State newstate )
{
  if ( newstate!=m_state )
    debugI("!Register manager state chg %s => %s", StateText(m_state), StateText(newstate) );

  switch ( m_state ) 
  {
      case STATE_CREATED:
                break;
      case STATE_DISABLED:
                break;
      case STATE_CONNECTING:
                break;
      case STATE_CONNECTED:
                break;
      
      default:  break;
  }; 

  m_state = newstate;

  uint32_t ms = millis();
  if ( m_reportLastMs==0 || (ms-m_reportLastMs>10000 ) )
  {
    debugI("\nRegister manager state %s, core %d", stateText(), OSAPI_Task::GetCurrentCoreId() );
    m_reportLastMs = ms;
  }


}

/* *************************************************** */
bool RegisterManager::updateRegisters()
{
  uint32_t now = millis();

  bool access_err = false;

  for ( int32_t regindex=0; regindex < m_registers.size(); regindex++ )
  {
    Register* reg = m_registers.at( regindex );
    if( reg->needsUpdate(now)==false )
      continue;

    //debugI("ix %d: Reg %d age %dms", reg->getStart(), now - reg->getLastUpdated() );


    bool access_ok = false;

    if(reg->isCoil()) 
      access_ok = readCoil( reg );
    else 
      access_ok = readRegister( reg );

    if ( access_ok )
    {
      reg->setLastUpdated( now );
    }
    else
    {
      access_err = true;
      break; // don't try to read all if problems
    }

    yield();
  }

  return (access_err==false);
}

/* *************************************************** */
bool RegisterManager::readRegister(Register *reg) 
{

  int start = reg->getStart(); // start address of register block.
  int len = reg->getLength();  // number of single registers in block.
  uint32_t start_t = millis();

  uint8_t result; 
  const int max_tries = 2;
  for ( int i=0; i<max_tries; i++ ) // read retry loop
  {
    start_t = millis(); // set for each retry.
    // Perform MODBUS read transaction access.
    m_modbus.clearResponseBuffer();
    m_modbus.clearTransmitBuffer();

    result = m_modbus.readHoldingRegisters( start, len );

    debugD("%06u: rd-reg %5d,L=%2d %4s %4dms/d%4dms [%3u] r%u(%d,e%d)/w%u(%d,e%u)", start_t, start,len, result == m_modbus.ku8MBSuccess ? "OK":"FAIL", 
                millis()-start_t, start_t - m_lastFailedRead,
                result, 
                m_numSuccessfulReads, m_numReadRetries,m_numFailedReads,
                m_numSuccessfulWrites, m_numWriteRetries, m_numFailedWrites );
    
    if( result == m_modbus.ku8MBSuccess) 
      break; // break from retry loop on sucess

    // TODO: see if any error code could indicate it's not worth trying again.
    //       also check if timeout on modbus could be set lower, currently its a private static 2sec in the library.

    m_numReadRetries++;
    if ( i != max_tries-1 )
      Sleep(10); // small retry wait.
  }

  // Handle read error.
  if( result != m_modbus.ku8MBSuccess ) 
  {
    m_lastFailedRead = start_t;
    m_numFailedReads++;
    return false;
  }

  // read success
  // move read values into single register value caches and update change flags.
  m_numSuccessfulReads++;
  
  char cr[80]; int crlen=0; cr[0]=cr[sizeof(cr)-1]=0; // malloc free change string

  for (int i = 0; i< len; i++ )
  {
    int address = start + i + 1;
    const String &name = reg->getRegisterName(address);
    
    if(reg->isReadable(address) && name != "") 
    {
        short update = (short) m_modbus.getResponseBuffer(i); // get read value.
        
        // Store value in cache, flag if value was updated.
        if( reg->setValue(address, (int) update) ) 
        {
          crlen += snprintf(&cr[crlen], sizeof(cr)-1-crlen,"%d ", address); // append in a cheap way.
          // debugD(" %d changed!", address );
        }
    }
  }

  if ( crlen > 0 )  debugI(" changed: %s", cr); // print what registers were changed.

  m_lastSuccessfulRead = millis();
  return true;
}
 

/* *************************************************** */
bool RegisterManager::readCoil(Register *reg) 
{

  int start = reg->getStart();
  int len = reg->getLength();
  uint32_t start_t = millis();

  uint8_t result; 
  const int max_tries = 2;
  for ( int i=0; i<max_tries; i++ ) // read retry loop
  {
    start_t = millis();

    m_modbus.clearResponseBuffer();
    m_modbus.clearTransmitBuffer();

    // Perform the MODBUS transaction to get coil set.
    uint8_t result = m_modbus.readCoils(start, len);

    debugD("%06u: rd-coi %5d,L=%2d %4s %4dms/d%4dms [%3u] r%u(%d,e%d)/w%u(%d,e%u)", start_t, start,len, result == m_modbus.ku8MBSuccess ? "OK":"FAIL", 
            millis()-start_t,start_t - m_lastFailedRead, result, 
            m_numSuccessfulReads, m_numReadRetries,m_numFailedReads,
            m_numSuccessfulWrites, m_numWriteRetries, m_numFailedWrites );

    if( result == m_modbus.ku8MBSuccess) 
      break; // break from retry loop on sucess


    m_numReadRetries++;
    if ( i != max_tries-1 )
      Sleep(10); // small retry wait.
  }

  if(result != m_modbus.ku8MBSuccess) 
  {
    m_lastFailedRead = start_t;
    m_numFailedReads++;
    return false;
  }
  
  
  m_numSuccessfulReads++;

  // malloc free change stats string, show up to 80 chars truncating.
  char cr[80]; int crlen=0; cr[0]=cr[sizeof(cr)-1]=0;

  for(int i = 0; i < len;) {
      uint16_t raw = m_modbus.getResponseBuffer(i/16);
      while(i < len) {
          int address = start + i + 1;
          const String &name = reg->getRegisterName(address);
          if(reg->isReadable(address) && name != "") {
              int update = raw & 0x1;
              if(reg->setValue(address, update)) {
                crlen += snprintf(&cr[crlen], sizeof(cr)-1-crlen,"%d ", address); // append in a cheap way.
              }
          }
          i++;
          if(i%16 == 0)
              break;
          raw = raw >> 1;
      }
  }

  if ( crlen > 0 )  debugI(" changed: %s", cr); // print what registers were changed.

  m_lastSuccessfulRead = millis();
  return true;

}

/************************************************************************
// Visitor passed through registry manager to commit pending writes
// to Villavent through MODBUS writes. 
// Clears the pending write flag on success. New values will typically
// be read out soon thereafter, acking on MQTT
/************************************************************************/
class PendingWriteVisitor : public RegisterManager::Visitor
{
    RegisterManager &m_manager;
    ModbusMaster    &m_modbus;
    uint32_t         m_written; // num writes.
    uint32_t         m_failures; // failes on all retries.
public:
    PendingWriteVisitor(RegisterManager &m, ModbusMaster &modbus) 
      : Visitor(0,FLG_VISIT_REG_SINGLE|FLG_VISIT_WRITE_PENDING),
        m_manager(m), m_modbus(modbus),
         m_written(0), m_failures(0) {}
 
    uint32_t numWrites() const { return m_written; }
    uint32_t numFailures() const { return m_failures; }

    // Implement in your visitor class and return negative to
    // interrupt traverse.
    virtual int32_t visit( Register &reg ) { return 0; };
    virtual int32_t visit( int32_t address, Register &reg );;
};


/* *************************************************** */
int32_t PendingWriteVisitor::visit( int32_t address, Register &reg )
{
  if ( reg.hasPendingWrite(address)== false ) // uneeded extra check..
    return 0;

  int32_t update = reg.getPendingWriteValue( address );

  uint8_t result=m_modbus.ku8MBSuccess;

  const int max_tries = 2;
  for (int i = 0; i < max_tries; i++) // retry loop.
  {
    uint32_t start_t = millis();
    bool is_coil = reg.isCoil();

    m_modbus.clearResponseBuffer();
    m_modbus.clearTransmitBuffer();

    uint8_t result;
    if ( is_coil )
      result = m_modbus.writeSingleCoil(address-1, update);
    else
      result = m_modbus.writeSingleRegister(address-1, update);

    debugE("%06u: wr-%-4s %5d,L=%2d %4s %4dms [%3u] r%u/w%u(%d,e%d)", start_t, is_coil?"coil":"reg", address-1,1, 
            result == m_modbus.ku8MBSuccess ? "OK":"FAIL", millis()-start_t, result, 
            m_manager.m_numSuccessfulReads, m_manager.m_numSuccessfulWrites, m_manager.m_numWriteRetries, m_manager.m_numFailedWrites );

    if( result == m_modbus.ku8MBSuccess) {

      reg.confirmPendingWrite( address ); // clear the pending write flag.

      reg.setLastUpdated( millis() - reg.getInterval() ); // trick - date back to assure readback fast.
      
      m_manager.m_numSuccessfulWrites++;

      m_written++;

      //  sendMqttMessage(name, formatted); // this will happen as a feed-back response when value is read back?
      break; // break from retry loop.
    }

    m_manager.m_numWriteRetries++;

    // Sleep between re-tries.
    if ( i != max_tries-1 )
      OSAPI_Task::Sleep(30); // small retry wait.
  }

  if ( result != m_modbus.ku8MBSuccess )
  {
    m_failures++;
    m_manager.m_numFailedWrites++;
    return -1; // Break accept traversal if a register fails totally
  }
  return 0;
   

}

/* *************************************************** */
bool RegisterManager::processPendingCommits()
{
   PendingWriteVisitor visitor( *this, this->m_modbus ) ;

   // Pass this visitor throug nodes that have pending writes.
   int32_t res = acceptNoProtect( visitor );

   debugD("Performed %d MODBUS writes, %u failures", visitor.numWrites(), visitor.numFailures() );

   return visitor.numFailures()!=0 ? false : true;
}

/* *************************************************** */
const char *RegisterManager::StateText(State state) 
{
  switch ( state ) {
    case STATE_CREATED: return "Created";
    case STATE_DISABLED: return "Disabled";
    case STATE_CONNECTING: return "Connecting";
    case STATE_CONNECTED: return "Connected";
    default: return "Unknown";
  }
}

/* *************************************************** */
int32_t RegisterManager::Visitor::getIncIndex( int32_t last_valid_index )
{
  int32_t curr = currIndex();
  if ( curr > last_valid_index ) // allready higher than allowed (should not happen under normal use.)
  {
    resetIndex();
    curr = currIndex();
  }

  m_curr_idx++;
  if ( m_curr_idx > last_valid_index ) // already higher than allowed (should not happen)
    resetIndex();

  return curr;
}

/* *************************************************** */
int32_t RegisterManager::acceptNoProtect(Visitor &visitor)
{
  const uint32_t count = m_registers.size();
  uint32_t i = 0;

  int32_t visitor_res = 0;

   // debugI("Accept traverse count %d start at %d", count, visitor.currIndex() );

  // visit each reg at most once. Break if visitor returns negative.
  for (  i=0; visitor_res>=0 && i<count; i++ ) 
  {
    // traverse starting with value provided by visitor.
    int32_t regindex = visitor.getIncIndex( count-1 );

    Register* reg = m_registers.at(regindex);
    if ( !reg )
      continue;
    
    if ( visitor.getFlags(Visitor::FLG_VISIT_REG_BLOCK) )
    {
      visitor_res = visitor.visit( *reg );
      if ( visitor_res < 0 )
        break; // break out of main loop if visitor returns negative.
    } 

    if ( visitor.getFlags(Visitor::FLG_VISIT_REG_SINGLE)==0 )
      continue; // visitor not set up to visit single registers.

    int32_t start = reg->getStart();
    int32_t len =   reg->getLength();  // number of single registers/coils in block

    for ( int j=0; visitor_res>=0 && j<len; j++ )
    {
      int address = start + j + 1; // calculate register address value.

      bool updated = reg->hasUpdatedValue(address); // whether a new values is available.
      bool pnd_wr = reg->hasPendingWrite(address);  // whether there is a pending write on register.

      if ( (visitor.getFlags(Visitor::FLG_VISIT_WRITE_PENDING) && pnd_wr) ||
           (visitor.getFlags(Visitor::FLG_VISIT_UPDATED) && updated) ||
           (visitor.getFlags(Visitor::FLG_VISIT_UNCHANGED) ) )
      {
        visitor_res = visitor.visit( address, *reg );
        if ( visitor_res < 0 )
          break;
      }

    }
  }

  return i;

}

/* *************************************************** */
int32_t RegisterManager::accept( Visitor &visitor, int32_t timeout_ms )
{
  // LOck semaphore, suspend untill available or timeout.
  int32_t res = m_sem.take( timeout_ms ); 
  if ( res < 0 )
    return res;

  res = acceptNoProtect( visitor );

  m_sem.give(); // let go of semaphore.

  return res;

}

/* *************************************************** */
Register* RegisterManager::findRegisterByAddress( int32_t address)
{
  for(int i = 0; i < m_registers.size(); i++ ) 
  {
    Register* reg = m_registers.at(i);
    if ( address > reg->getStart() && address < (reg->getStart()+reg->getLength()) )  
      return reg;
  }

  return 0;
}


/* *************************************************** */
Register* RegisterManager::findRegisterAndAddressByName( const String &name, int32_t &address )
{
  for(int i = 0; i < m_registers.size(); i++ ) 
  {
    Register* reg = m_registers.at( i );

    address = reg->getRegisterAddress( name );
    if ( address!=REG_INVALID )
      return reg;  
  }
  return 0;
}


/* *************************************************** */
int32_t RegisterManager::setPendingWriteByAddress( int32_t address, const String &value)
{
  bool set = false;
  int32_t res = -1;

  Register* reg = findRegisterByAddress( address );
  if ( !reg )
    return res;

  //int current = reg->getValue( address );
  
  if(!reg->isWriteable(address))
    res = -2;
  else if ( reg->setFormattedValue(address, value, true ) )
  {
    set = true;
    res = 1;
  }
  else
    res = 0;

  

  #if SLAVE_MODE
    set = reg->setFormattedValue(address, value);
  #endif

  return res;
}
  
/* *************************************************** */
int32_t RegisterManager::setPendingWriteByName(const String &name, const String &value )
{
  bool set = false;
  int32_t res = -1;
  int32_t address;

  Register* reg = findRegisterAndAddressByName( name, address );
  if ( !reg )
    return res;

  int current = reg->getValue( address );

  debugD("Found name %s at addr %d value %d", name.c_str(), address, current );

  if ( !reg->isWriteable(address) )
    res = -2;
  else if ( reg->setFormattedValue(address, value, true) ) // Set pending
  {
    set = true;
    res = 1;
  }
  else
  {
    res  = 0;
  }

  #if SLAVE_MODE
    set = reg->setFormattedValue(address, value);
  #endif

  return res;
}
