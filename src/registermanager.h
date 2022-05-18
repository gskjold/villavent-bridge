#ifndef __REGISTER_MANAGER__
#define __REGISTER_MANAGER__

#include "osapi.h"
//#include <LinkedList.h>  changed to std::vector, since linked list index lookup is slower compared to direct indexing in a vector. 

#include <ModbusMaster.h>

#include <vector>


class Register;

/**
 * @brief Thread responsible for all MODBUS register reading and writing
 * 
 * 
 */
class RegisterManager : public OSAPI_Task
{
public:
  enum State {
    STATE_CREATED        = 1, 
    STATE_DISABLED       = 2,      
    STATE_CONNECTING     = 4,
    STATE_CONNECTED      = 5
  };

  enum Constants {
     DEFAULT_PRI = 1,
     STACKSIZE   = 6000
   };

  RegisterManager( ModbusMaster &master );
  ~RegisterManager();

  // Do at init, before enabling manager
  void addRegisters();

  void enable();
  void disable();


  // Set a value in the cache to be written at first opportunity by the manager
  // address: any valid register address within any block.
  int32_t setPendingWriteByAddress( int32_t address, const String &value);
  
  // Set a value in the cache to be written at first opportunity by the manager
  // name: mqttname, value from payload.
  int32_t setPendingWriteByName(const String &name, const String &value );


  // get name of current state.
  const char *stateText() const { return StateText(m_state); } 
  static const char* StateText( State state );

  // TODO : THIS SHOULD REALLY NOT EXIST:
  //LinkedList<Register*> &registers() { return m_registers; }
  std::vector<Register*> &registers() { return m_registers; }

  // Ms of last successfull read.
  uint32_t lastSuccessfulRead() const { return m_lastSuccessfulRead; }



  //
  // Class used to visit register entries while holding the 
  // semaphore to assure mutual acces.
  // User inherits and implements the visit method
  //
  class Visitor 
  {
  public:
    // Flags 
    enum FilterMasks { 
      FLG_VISIT_REG_BLOCK     = 0x01,
      FLG_VISIT_REG_SINGLE    = 0x02,
      FLG_VISIT_UNCHANGED     = 0x04, 
      FLG_VISIT_UPDATED       = 0x08,
      FLG_VISIT_WRITE_PENDING = 0x10,
      FLG_VISIT_ALL= FLG_VISIT_REG_BLOCK|FLG_VISIT_REG_SINGLE|FLG_VISIT_UNCHANGED|FLG_VISIT_UPDATED|FLG_VISIT_WRITE_PENDING 
    };

    // Visitor. start_idx can be set to start from previous value to implement
    // round robin visiting even though not visiting all nodes, by returning <0 
    // in visit() methods.
    Visitor(uint32_t start_idx=0, uint8_t flags = FLG_VISIT_ALL): m_curr_idx(start_idx), m_flags(flags) {}
    virtual ~Visitor(){};

    uint8_t getFlags( uint8_t mask  ) const { return m_flags & mask; }
    bool    doVisitUnchanged()        const { return getFlags(FLG_VISIT_UNCHANGED); }
    bool    doVisitUpdated()          const { return getFlags(FLG_VISIT_UPDATED);}
    bool    doVisitWritePending()     const { return getFlags(FLG_VISIT_WRITE_PENDING);}
    

    // Implement in your visitor class and return negative to
    // interrupt traverse.
    virtual int32_t visit( Register &reg ) { return 0; };

    // Implement this to visit individual single registers
    ///return negative will interrupt traverse.
    virtual int32_t visit( int32_t address, Register &reg ) { return 0; };

    int32_t currIndex() const { return m_curr_idx; }
    void    resetIndex() { m_curr_idx=0; }
    // inc and get value before inc. If prev val > max, start from 0 and let next be 1.
    int32_t getIncIndex( int32_t last_valid_index );

  protected:
    int32_t m_curr_idx;
    uint8_t m_flags;
  
  }; // End VISITOR API class.


  // Implements the visitor pattern. https://en.wikipedia.org/wiki/Visitor_pattern
  // Object is passed thorough internal data structure.
  // while semaphore is helt.
  //
  int32_t accept( Visitor &visitor, int32_t timeout_ms = -1 );

private:
  int32_t protect( int32_t mstimeout ); // take semaphore
  void unprotect();                 // release semaphore

  void switchState( State newstate );

  virtual void taskCode();

  bool processPendingCommits();

  bool updateRegisters();
  bool readRegister(Register *reg);
  bool readCoil(Register *reg);

  Register* findRegisterByAddress( int32_t addresse );
  Register* findRegisterAndAddressByName( const String &name, int32_t &address );

  int32_t acceptNoProtect( Visitor &visitor);

  void clearStats();

private:
  OSAPI_Semaphore        m_sem;
  bool                   m_enabled;
  State                  m_state;
  std::vector<Register*> m_registers; // changed from LinkeList since this is better, direct lookup array.
  ModbusMaster          &m_modbus;

  // Couple of statistics counters.
  uint32_t m_lastSuccessfulRead; // ms
  uint32_t m_lastFailedRead;     // ms

  uint32_t m_numSuccessfulReads;
  uint32_t m_numSuccessfulWrites;
  uint32_t m_numFailedReads;
  uint32_t m_numFailedWrites;
  uint32_t m_numReadRetries;
  uint32_t m_numWriteRetries;

  uint32_t m_reportLastMs; // used for regular status print.


  friend class PendingWriteVisitor;
};



#endif // sentinel