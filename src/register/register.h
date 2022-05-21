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

public:
    enum Flags
    {
       FLG_VALUE_UPDATED = 0x01,
       FLG_PENDING_WRITE = 0x02,
       FLG_MQTT_RETAIN   = 0x04
    };

    // *****************************************************************************    
    // Context info kept for single registers. Only readables public to others than
    // Register class.
    class SingleReg
    {
    public:
       SingleReg() : m_value(VAL_INVALID), m_pending_set_value(VAL_INVALID),m_perm(0),m_flags(0),m_mqtt_period_256ms(0),m_mqtt_timestamp_256ms(0){}

       // Read, typically in a visitor, whether register is due for MQTT export, i.e
       // the value is updated compared to the previously exported, and any throttling
       // time has passed too.
       // PS: visitors can also be flagged to only visit registers that fulfill this 
       //     check, i.e let the RegistryManager check before allowing the visit.
       bool     readyForMqttExport( uint32_t ms_now ) const;

       // Get MQTT retain flag for single-register. Typically, frequently updated 
       // sensor values may not have retain set, while slow updating fields would 
       // use retain. Also retain flags may need overriding for test purposes.
       bool     mqttRetain() const { return getFlags(FLG_MQTT_RETAIN)!=0 ? true : false; }
 
    protected:
        // Throttle export interval in addition to the polling period for the register block.
        // This can be used if a block is polled for fast response on setting registers, while 
        // not wanting to be floded by continously changing measurement registers.
        // To save memory and ease calcs period is set in resolution of 256ms so we can shift 
        // the millisecond counter 8 bits down and look at the next 16 next bits.
        // storing period as 16 bits yields a max throttle period of 65536*256ms = 16776s = 4.6hrs
        void     setMqttThrottlePeriod256ms( uint16_t period_256ms ) { m_mqtt_period_256ms=period_256ms; }
        void     setMqttThrottlePeriodMs( uint32_t ms ) { setMqttThrottlePeriod256ms( MsTo256ms(ms) ); }
        uint16_t getMqttThrottlePeriod256ms( uint16_t period ) const { return m_mqtt_period_256ms; }
        uint16_t setMqttExportTimestamp( uint32_t ms_now ) { m_mqtt_timestamp_256ms = MsTo256ms(ms_now); return m_mqtt_timestamp_256ms; }

        static uint16_t MsTo256ms(uint32_t ms ) { return (uint16_t) ((ms>>8) & 0xffff); }

        // set flags, return the ones that were previously 0.
        uint8_t setFlags( uint8_t flags );
        // clear flags return the ones that were previusly 1.
        uint8_t clearFlags( uint8_t flags );
        uint8_t getFlags(uint8_t flags=0xff) const { return m_flags & flags; }


    private:
        String     m_name;
        int32_t    m_value;
        int32_t    m_pending_set_value;
        uint8_t    m_perm;
        uint8_t    m_flags;    
        uint16_t   m_mqtt_period_256ms;
        uint16_t   m_mqtt_timestamp_256ms;

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

    // Use this method to set a new value to be written by the registry manager.
    // returns true if there was a change.
    bool    setNewPendingWriteValue( int32_t address, int32_t value );
    
    // check if there is a pending write request 
    bool    hasPendingWrite(int32_t address) const;
    
    // Get value of pending write.
    int32_t getPendingWriteValue( int32_t address ) const;
    
    // Clear pending write flag
    void confirmPendingWrite( int32_t address );


    // Check if value is updated. Returns flag, but also returns
    // in ready_for_export whether throttling indicates whether to send MQTT
    // given the ms timestamp now.
    bool hasUpdatedValue(int32_t address, const SingleReg **sreg_pp=0) const;  // check update flag.
    void confirmUpdate(int32_t address, uint32_t millis);    // acknowledge the updated value.
    int getValue(int address) const;              // get latest read value.


    /* add a single register definition. 
     @param address 
                Address from MODBUS spec SystemAir Villavent
     @param name
                Topic name on MQTT.
     @param permission
                Read/Write/FUll permission.
     @param mqtt_retain
                Whether retain flag should be used when posting MQTT messages for this parameter.
                Later a global override rule may be added if required
     @param mqtt_throttle_time_ms
                Throtling of frequency of MQTT export of this parameter.
                For space/performance has a resolution of 256ms steps, if you must
                specify at least 256ms, and e.g. 1000ms yields (1000/256)*256 = (1000>>8)*256 = 768ms
                Also beware that the polling time of the register block containing the register will
                influence how old the value exported could be, but you always get the latest sampled
                value.
                If having a MODBUS poll period of 500ms on the register block you can choose to set
                throttle time on some sub-registers higher, but not lower, to reduce the frequency of
                getting continous measurements, f.ex for the fan speed and temperature readings, 
                that jump up and down and cause most of the traffic. 
                Some user may prefer a higher  interval to reduce storage space in logging databases.
    */
    void addRegister(int address, const String &name, byte permission=PERM_FULL, bool mqtt_retain=true, uint32_t mqtt_throttle_time_ms=0 );
    const String &getRegisterName(int address) const;
    int getRegisterAddress(const String &name) const;
    bool isReadable(int address) const;
    bool isWriteable(int address) const;

    virtual String getFormattedValue(int address) const;
    virtual boolean setFormattedValue(int address, const String &value, bool setpending=false);

    // Check if register block needs a new poll on Modbus.
    boolean  needsUpdate(uint32_t millis) const;
    // Set timetamp of poll in milliseconds.
    void     setLastUpdated(uint32_t millis);
    // Get timestamp of last poll in milliseconds.
    uint32_t getLastUpdated() const { return m_last_updated; }
    // Get configured interval for polling of register block.
    int32_t  getInterval() const { return m_interval; }    

protected:
    // Set the value in the cache for a given register address. This is 
    // typically only done by the RegisterManager after reading a new value on Modbus.
    boolean setValue(int address, int value); // set value, return true if changed.

    // Map from Modbus address to index to Register block containing that register address.
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
