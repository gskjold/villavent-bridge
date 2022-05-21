#include "Arduino.h"
#include "Update.h"

#include "main.h"
#include "debugger.h"

#include <ModbusMaster.h>


#include "configuration.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <lwip/apps/sntp.h>

#include <MQTT.h>
#include "web/InternalWebServer.h"
#include "HwTools.h"

#include "registermanager.h"
#include "osapi.h"

ModbusMaster    Modbus;
RegisterManager RegisterManager( Modbus );


HwTools hw;
configuration config;
SystemConfig sysConfig;
GpioConfig gpio = {
    4,
    5,
    true,
    0xFF,
    0xFF,
    0xFF,
    true,
    0xFF,
    0,
    1000,
    0
};
DNSServer* dnsServer = NULL;


WiFiClient *client;
MQTTClient mqtt(128);
bool mqttEnabled = false;
MqttConfig* mqttConfig = NULL;

bool ntpEnabled = false;

InternalWebServer ws(&hw);

void preTransmission()
{
  //debugD( "Write start");
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
//  Serial2.flush(); // already done in driver.
  //delay( 3 );
  // delayMicroseconds(120);
  //delayMicroseconds(120);

  digitalWrite(MAX485_DE, 0);
  digitalWrite(MAX485_RE_NEG, 0);

  // debugD("End of write");
}

TaskHandle_t Task1;

void Task1code( void * parameter) {
  for(;;) {
    ws.loop();
    
    vTaskDelay(1);
  }
}

void setup() {
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(115200);

  pinMode(gpio.apPin, INPUT_PULLUP);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  debugI("Booting (Main thread priority %d core %d)", OSAPI_Task::GetTaskPriority(), OSAPI_Task::GetCurrentCoreId() );

  hw.setup(&gpio);
  
 

//  for ( uint8_t cpu=0; cpu<2; cpu ++ )
//  {
//    TaskHandle_t t = xTaskGetIdleTaskHandleForCPU( cpu );
//    if ( t )
//    {

//    }

//  }

  if(SPIFFS.begin(true)) {
    bool flashed = false;
    if(SPIFFS.exists(FILE_FIRMWARE)) {
        debugI("Found firmware");

        debugI(" flashing");
        File firmwareFile = SPIFFS.open(FILE_FIRMWARE, "r");
        debugD(" firmware size: %d\n", firmwareFile.size());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        debugD(" available: %d\n", maxSketchSpace);
        if (!Update.begin(maxSketchSpace, U_FLASH)) {
            #if DEBUG_MODE
                debugE("Unable to start firmware update");
                Update.printError(Serial);
            #endif
        } else {
            while (firmwareFile.available()) {
                uint8_t ibuffer[128];
                firmwareFile.read((uint8_t *)ibuffer, 128);
                Update.write(ibuffer, sizeof(ibuffer));
            }
            flashed = Update.end(true);
        }
        firmwareFile.close();
        SPIFFS.remove(FILE_FIRMWARE);
    }
    SPIFFS.end();
    if(flashed) {
        debugI("Firmware update complete, restarting");
        Serial.flush();
        delay(250);
        ESP.restart();
        return;
    }
  }

  debugI("Adding registers");
  RegisterManager.addRegisters();

  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  if(config.hasConfig()) {
    debugI("Loading configuration");
    config.getSystemConfig(sysConfig);
    WiFi_connect();
    
    NtpConfig ntp;
    if(config.getNtpConfig(ntp)) {
        debugD("Configuring NTP");
        configTime(ntp.offset*10, ntp.summerOffset*10, ntp.enable ? ntp.server : "");
        sntp_servermode_dhcp(ntp.enable && ntp.dhcp ? 1 : 0);
        ntpEnabled = ntp.enable;
        //TimeChangeRule std = {"STD", Last, Sun, Oct, 3, ntp.offset / 6};
        //TimeChangeRule dst = {"DST", Last, Sun, Mar, 2, (ntp.offset + ntp.summerOffset) / 6};
        //tz = new Timezone(dst, std);
        //ws.setTimezone(tz);
    }

    MqttConfig mc;
    config.getMqttConfig(mc);
    mqttEnabled = strlen(mc.host) > 0;
  } else {
    debugW("No config, booting AP");
    swapWifiMode();
  }

  Stream* debugger = NULL;
  #if DEBUG_MODE
    debugger = &Serial;
  #endif

  debugI("Setting up web server");
  ws.setup(&config, &mqtt, &RegisterManager, debugger);
  ws.setMessageHandler(mqttMessageReceived);

//  xTaskCreatePinnedToCore(
//      Task1code, /* Function to implement the task */
//      "Webserver", /* Name of the task */
//      10000,  /* Stack size in words */
//      NULL,  /* Task input parameter */
//      -1,  /* Priority of the task */
//      &Task1,  /* Task handle. */
//      0); /* Core where the task should run */

   debugI("Starting register manager thread");

   RegisterManager.start(); // Start the register manager thread once.

}

unsigned long wifiTimeout = WIFI_CONNECTION_TIMEOUT;
unsigned long lastWifiRetry = -WIFI_CONNECTION_TIMEOUT;
void WiFi_connect() {
	if(millis() - lastWifiRetry < wifiTimeout) {
		delay(50);
		return;
	}
	lastWifiRetry = millis();

	if (WiFi.status() != WL_CONNECTED) {
		WiFiConfig wifi;
		if(!config.getWiFiConfig(wifi) || strlen(wifi.ssid) == 0) {
			swapWifiMode();
			return;
		}

		debugI("Connecting to WiFi network: %s", wifi.ssid);

		MDNS.end();
		WiFi.disconnect();
		yield();

		WiFi.enableAP(false);
		WiFi.mode(WIFI_STA);
		if(strlen(wifi.ip) > 0) {
			IPAddress ip, gw, sn(255,255,255,0), dns1, dns2;
			ip.fromString(wifi.ip);
			gw.fromString(wifi.gateway);
			sn.fromString(wifi.subnet);
			dns1.fromString(wifi.dns1);
			dns2.fromString(wifi.dns2);

			WiFi.config(ip, gw, sn, dns1, dns2);

            debugI("Using configured IP: %s", wifi.ip );
		} else {
            debugI("No configured IP, DHCP?" );

			// WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // Workaround to make DHCP hostname work for ESP32. See: https://github.com/espressif/arduino-esp32/issues/2537
		}
		if(strlen(wifi.hostname) > 0) {
			WiFi.setHostname(wifi.hostname);
		}
		WiFi.begin(wifi.ssid, wifi.psk);
		yield();
	}
}

void swapWifiMode() {
	if(!hw.ledOn(LED_YELLOW)) {
		hw.ledOn(LED_INTERNAL);
	}
	WiFiMode_t mode = WiFi.getMode();
	if(dnsServer != NULL) {
		dnsServer->stop();
	}
	WiFi.disconnect(true);
	WiFi.softAPdisconnect(true);
	WiFi.mode(WIFI_OFF);
	yield();

	if (mode != WIFI_AP || !config.hasConfig()) {
		debugI("Swapping to AP mode, IP %s", WiFi.softAPIP().toString().c_str() );
		WiFi.softAP("VillaventBridge");
		WiFi.mode(WIFI_AP);

		if(dnsServer == NULL) {
			dnsServer = new DNSServer();
		}
		dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
		dnsServer->start(53, "*", WiFi.softAPIP());
	} else {
		debugI("Swapping to STA mode");
		if(dnsServer != NULL) {
			delete dnsServer;
			dnsServer = NULL;
		}
		WiFi_connect();
	}
	delay(500);
	if(!hw.ledOff(LED_YELLOW)) {
		hw.ledOff(LED_INTERNAL);
	}
}

unsigned long lastMqttRetry = -10000;
void MQTT_connect() {
    if(mqttConfig != NULL)
        delete mqttConfig;
    mqttConfig = new MqttConfig();

	if(!config.getMqttConfig(*mqttConfig) || strlen(mqttConfig->host) == 0) {
		debugW("No MQTT config");
		mqttEnabled = false;
		ws.setMqttEnabled(false);
		config.ackMqttChange();
		return;
	}
	if(millis() - lastMqttRetry < (mqtt.lastError() == 0 || config.isMqttChanged() ? 5000 : 30000)) {
		yield();
		return;
	}
	lastMqttRetry = millis();

	mqttEnabled = true;
	ws.setMqttEnabled(true);

    debugI("Disconnecting MQTT before connecting");

	mqtt.disconnect();
	yield();

	WiFiClientSecure *secureClient = NULL;
	Client *client = NULL;
	if(mqttConfig->ssl) {
		debugI("MQTT SSL is configured");

		secureClient = new WiFiClientSecure();

		if(SPIFFS.begin()) {
			char *ca = NULL;
			char *cert = NULL;
			char *key = NULL;

			if(SPIFFS.exists(FILE_MQTT_CA)) {
				debugI("Found MQTT CA file");
				File file = SPIFFS.open(FILE_MQTT_CA, "r");
				secureClient->loadCACert(file, file.size());
			}
			if(SPIFFS.exists(FILE_MQTT_CERT)) {
				debugI("Found MQTT certificate file");
				File file = SPIFFS.open(FILE_MQTT_CERT, "r");
				secureClient->loadCertificate(file, file.size());
			}
			if(SPIFFS.exists(FILE_MQTT_KEY)) {
				debugI("Found MQTT key file");
				File file = SPIFFS.open(FILE_MQTT_KEY, "r");
				secureClient->loadPrivateKey(file, file.size());
			}
			SPIFFS.end();
		}
		client = secureClient;
	} else {
		client = new WiFiClient();
	}

    debugI("Connecting to MQTT %s:%d", mqttConfig->host, mqttConfig->port);
	mqtt.begin(mqttConfig->host, mqttConfig->port, *client);

	// Connect to a unsecure or secure MQTT server
	if ((strlen(mqttConfig->username) == 0 && mqtt.connect(mqttConfig->clientId)) ||
		(strlen(mqttConfig->username) > 0 && mqtt.connect(mqttConfig->clientId, mqttConfig->username, mqttConfig->password))) {
		debugI("Successfully connected to MQTT!");
		config.ackMqttChange();

		// Subscribe to the chosen MQTT topic, if set in configuration
		if (strlen(mqttConfig->subscribeTopic) > 0) {
            mqtt.onMessage(mqttMessageReceived);
			mqtt.subscribe(String(mqttConfig->subscribeTopic) + "/#");
			debugI("  Subscribing to [%s]\r\n", mqttConfig->subscribeTopic);
		}
	} else {
        debugE("Failed to connect to MQTT");
	}
	yield();
}


/************************************************************************
// Visitor passed through registry manager to send MQTT messages for 
// updated registry entries. Visitor set up to visit only single registers
// that are updated.
/************************************************************************/
class MQTTUpdateVisitor : public RegisterManager::Visitor
{
    uint32_t m_messages_sent; // num MQTT messages sent.
public:
    MQTTUpdateVisitor() : Visitor(0,FLG_VISIT_REG_SINGLE|FLG_VISIT_UPDATED), m_messages_sent(0) {}
    //~MQTTUpdateVisitor();

    uint32_t numMessagesSent() const { return m_messages_sent; }
    void resetNumMessagesSent() { m_messages_sent=0; }

    // Implement in your visitor class and return negative to
    // interrupt traverse.
    virtual int32_t visit( Register &reg ) { return 0; };
    virtual int32_t visit( int32_t address, Register &reg );;

};

// Called to visit updated nodes only, no need to check specifically
int32_t MQTTUpdateVisitor::visit( int32_t address, Register &reg)
{
    const String &name = reg.getRegisterName( address );
    String formatted = reg.getFormattedValue( address );

    debugI("visit %s", name.c_str());

    sendMqttMessage(name, formatted);

    //TODO : could send fail and we need to handle that ?

    // Clear updated flag - i.e signal that change has been handled.
    reg.confirmUpdate( address );

    m_messages_sent++;

    if ( m_messages_sent > 2 )
      return -1; // break traverse in accept.
    else
      return 0;
}


int buttonTimer = 0;
bool buttonActive = false;
unsigned long longPressTime = 5000;
bool longPressActive = false;

bool wifiConnected = false;

bool initDone = false;
int regindex = 0;

unsigned long lastErrorBlink = 0; 

int lastError = 0;
 
// VIsitor passed to RegistryManager to send MQTT updates for changed 
// registers.
MQTTUpdateVisitor MqttUpdateVisitor;



void loop() {
	unsigned long now = millis();
	if(gpio.apPin != 0xFF) {
		if (digitalRead(gpio.apPin) == LOW) {
			if (buttonActive == false) {
				buttonActive = true;
				buttonTimer = now;
			}

			if ((now - buttonTimer > longPressTime) && (longPressActive == false)) {
				longPressActive = true;
				swapWifiMode();
			}
		} else {
			if (buttonActive == true) {
				if (longPressActive == true) {
					longPressActive = false;
				} else {
					// Single press action
				}
				buttonActive = false;
			}
		}
	}
 
    if ( now % 3000 == 0 )
    {
       debugI("IP:  %s, Main Core %d", WiFi.localIP().toString().c_str(), OSAPI_Task::GetCurrentCoreId());
    }

	// Only do normal stuff if we're not booted as AP
	if (WiFi.getMode() != WIFI_AP) {
		if (WiFi.status() != WL_CONNECTED) {
			wifiConnected = false;
			WiFi_connect();
		} else {
			if(!wifiConnected) {
				wifiConnected = true;
				
				WiFiConfig wifi;
				if(config.getWiFiConfig(wifi)) {
                    debugI("Successfully connected to WiFi!");
                    debugI("IP:  %s", WiFi.localIP().toString().c_str());
                    debugI("GW:  %s", WiFi.gatewayIP().toString().c_str());
                    debugI("DNS: %s", WiFi.dnsIP().toString().c_str());

					if(strlen(wifi.hostname) > 0 && wifi.mdns) {
						debugD("mDNS is enabled, using host: %s", wifi.hostname);
						if(MDNS.begin(wifi.hostname)) {
							MDNS.addService("http", "tcp", 80);
						} else {
							debugE("Failed to set up mDNS!");
						}
					}
				}
			}
			if(config.isNtpChanged()) {
				NtpConfig ntp;
				if(config.getNtpConfig(ntp)) {
					configTime(ntp.offset*10, ntp.summerOffset*10, ntp.enable ? ntp.server : "");
					sntp_servermode_dhcp(ntp.enable && ntp.dhcp ? 1 : 0);
					ntpEnabled = ntp.enable;

                    // TODO
					//if(tz != NULL) delete tz;
					//TimeChangeRule std = {"STD", Last, Sun, Oct, 3, ntp.offset / 6};
					//TimeChangeRule dst = {"DST", Last, Sun, Mar, 2, (ntp.offset + ntp.summerOffset) / 6};
					//tz = new Timezone(dst, std);
					//ws.setTimezone(tz);
				}

				config.ackNtpChange();
			}

			if(now > 10000 && now - lastErrorBlink > 3000) {
				errorBlink();
			}

			if (mqttEnabled || config.isMqttChanged()) {
				mqtt.loop();
				delay(10); // Needed to preserve power. After adding this, the voltage is super smooth on a HAN powered device
				if(!mqtt.connected() || config.isMqttChanged()) {
					MQTT_connect();
				}
			} else if(mqtt.connected()) {
				mqtt.disconnect();
			}
            ws.loop();
		}
	} else {
		if(dnsServer != NULL) {
			dnsServer->processNextRequest();
		}
        ws.loop();

		// Continously flash the LED when AP mode
		if (now / 50 % 64 == 0) {
			if(!hw.ledBlink(LED_YELLOW, 1)) {
				hw.ledBlink(LED_INTERNAL, 1);
			}
		}
	}

    if(config.isSystemChanged()) {
        config.getSystemConfig(sysConfig);
        config.ackSystemChange();

        // Disable modbus accesses.
        RegisterManager.disable();

        if ( sysConfig.unitBaud > 0 && sysConfig.unitId > 0)  {
            debugI("Connecting to ventilation unit at baud %d", sysConfig.unitBaud);
            Serial2.begin( sysConfig.unitBaud );
            Modbus.begin( sysConfig.unitId, Serial2 );
            Modbus.preTransmission( preTransmission );
            Modbus.postTransmission( postTransmission );

            // enable background registry keeper and modbus accessor.
            RegisterManager.enable();
        }
 
    }

    if( sysConfig.unitBaud > 0 && sysConfig.unitId > 0 ) 
    {
       // Pass MQTT updater through updated registers to send messages.
       // block at most 300ms to wait for this.
       MqttUpdateVisitor.resetNumMessagesSent(); // clear stop-counter.
       
       uint32_t tstart = millis();
       int32_t cnt = RegisterManager.accept( MqttUpdateVisitor, 300 );

       if ( MqttUpdateVisitor.numMessagesSent()>0 )
         debugI("MQTT checked %d, %d updates sent  - %dms - core %d", cnt, MqttUpdateVisitor.numMessagesSent(), millis()-tstart, OSAPI_Task::GetCurrentCoreId() );
     
    }





    delay(1); // Allow modem sleep
}

void errorBlink() {
	if(lastError == 3)
		lastError = 0;
	lastErrorBlink = millis();
	for(;lastError < 3;lastError++) {
		switch(lastError) {
			case 0:
				if(lastErrorBlink - RegisterManager.lastSuccessfulRead() > 30000) {
					hw.ledBlink(LED_RED, 1); // If no message received on Modbus in 30 sec, blink once
					return;
				}
				break;
			case 1:
				if(mqttEnabled && mqtt.lastError() != 0) {
					hw.ledBlink(LED_RED, 2); // If MQTT error, blink twice
					return;
				}
				break;
			case 2:
				if(WiFi.getMode() != WIFI_AP && WiFi.status() != WL_CONNECTED) {
					hw.ledBlink(LED_RED, 3); // If WiFi not connected, blink three times
					return;
				}
				break;
		}
	}
}

void mqttMessageReceived(String &topic, String &payload) {
    debugI("Received message for topic %s", topic.c_str() );
    
    if(mqttConfig == NULL)
        return;

    int address = 0;
    int current = 0;
    Register* updateReg = NULL;

    int32_t setres = 0;

    if (topic.toInt() > 0) 
    {
        address = topic.toInt();

        setres = RegisterManager.setPendingWriteByAddress( address, payload );
    }
    else
    {
        if(!mqtt.connected() || !topic.startsWith(mqttConfig->subscribeTopic)) return;
        String trimmed = topic.substring(strlen(mqttConfig->subscribeTopic) + 1);
        debugD(" stripped away base topic gives us: %s", trimmed.c_str() );
        debugD(" payload is: %s", payload.c_str() );

        setres = RegisterManager.setPendingWriteByName( trimmed, payload );
    }

    debugD(" pending set = %d", setres );




}

void sendMqttMessage(const String &name, String &payload) {
    if(!mqtt.connected() || mqttConfig == NULL || strlen(mqttConfig->publishTopic) == 0) return;

    String topic = String(mqttConfig->publishTopic) + "/" + name;
    debugD("MQTT publish to %s with payload %s", topic.c_str(), payload.c_str());
    mqtt.publish(topic.c_str(), payload.c_str(), true, 0);
    mqtt.loop();
    yield();
}