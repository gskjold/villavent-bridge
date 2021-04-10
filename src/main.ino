#include "Arduino.h"
#include <LinkedList.h>

#include "main.h"
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

#include <ModbusMaster.h>

#include "configuration.h"
#include <WiFi.h>
#include <DNSServer.h>

#include <MQTT.h>
#include "web/InternalWebServer.h"

#define MAX485_DE      22
#define MAX485_RE_NEG  23

int currentId, currentBaud;
ModbusMaster node;

LinkedList<Register*> registers = LinkedList<Register*>();

configuration config;
DNSServer dnsServer;

WiFiClient *client;
MQTTClient mqtt(128);

InternalWebServer ws;

HardwareSerial* debugger = NULL;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
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

#if DEBUG_MODE
	debugger = &Serial;
#endif

  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  registers.add(new FanRegister());
  registers.add(new HeaterRegister());
  registers.add(new TempSensorStateCoil());
  registers.add(new RotorRegister());
  registers.add(new SystemRegister());
  registers.add(new NvmRegister());
  registers.add(new ClockRegister());
  registers.add(new FilterRegister());
  registers.add(new InputRegister());
  registers.add(new InputCoil());
  registers.add(new PcuPbCoil());
  registers.add(new AlarmCoil());
  registers.add(new AlarmRegister());

  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  if(config.hasConfig()) {
    config.load();
    WiFi_connect();
    client = new WiFiClient();
  } else {
    if(debugger) {
      debugger->println("No configuration, booting AP");
    }
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    WiFi.softAP("VillaventBridge");
    WiFi.mode(WIFI_AP);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }

  ws.setup(&config, debugger);

  xTaskCreatePinnedToCore(
      Task1code, /* Function to implement the task */
      "Webserver", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      -1,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */

}

unsigned long wifiTimeout = 30000;
unsigned long lastWifiRetry = -wifiTimeout;
void WiFi_connect() {
	if(millis() - lastWifiRetry < wifiTimeout) {
		delay(50);
		return;
	}
	lastWifiRetry = millis();

	if (debugger) {
		debugger->println();
		debugger->println();
		debugger->print("Connecting to WiFi network ");
		debugger->println(config.getWifiSsid());
	}

	if (WiFi.status() != WL_CONNECTED) {
		WiFi.disconnect();
		yield();

		WiFi.enableAP(false);
		WiFi.mode(WIFI_STA);
		if(!config.getWifiIp().isEmpty()) {
			IPAddress ip, gw, sn(255,255,255,0);
			ip.fromString(config.getWifiIp());
			gw.fromString(config.getWifiGw());
			sn.fromString(config.getWifiSubnet());
			WiFi.config(ip, gw, sn);
		}
		WiFi.begin(config.getWifiSsid().c_str(), config.getWifiPassword().c_str());
		yield();
	}
}

unsigned long lastMqttRetry = -10000;
void MQTT_connect() {
	if(config.getMqttHost().isEmpty()) {
		if(debugger) debugger->println("No MQTT config");
		return;
	}
	if(millis() - lastMqttRetry < 5000) {
		yield();
		return;
	}
	lastMqttRetry = millis();
	if(debugger) {
		debugger->print("Connecting to MQTT: ");
		debugger->print(config.getMqttHost());
		debugger->print(", port: ");
		debugger->print(config.getMqttPort());
		debugger->println();
	}

	mqtt.disconnect();
	yield();

	mqtt.begin(config.getMqttHost().c_str(), config.getMqttPort(), *client);

	// Connect to a unsecure or secure MQTT server
	if ((config.getMqttUser().isEmpty() && mqtt.connect(config.getMqttClientId().c_str())) ||
		(!config.getMqttUser().isEmpty() && mqtt.connect(config.getMqttClientId().c_str(), config.getMqttUser().c_str(), config.getMqttPassword().c_str()))) {
		if (debugger) debugger->println("\nSuccessfully connected to MQTT!");
		config.ackMqttChange();

		// Subscribe to the chosen MQTT topic, if set in configuration
		if (!config.getMqttSubscribeTopic().isEmpty()) {
            mqtt.onMessage(mqttMessageReceived);
			mqtt.subscribe(config.getMqttSubscribeTopic() + "/#");
			if (debugger) debugger->printf("  Subscribing to [%s]\r\n", config.getMqttSubscribeTopic().c_str());
		}
	} else {
		if (debugger) {
			debugger->print(" failed, ");
			debugger->println(" trying again in 5 seconds");
		}
	}
	yield();
}

bool initDone = false;
bool wifiConnected = false;
int regindex = 0;
void loop() {
    if(WiFi.getMode() == WIFI_AP) {
        dnsServer.processNextRequest();
    } else {
        // TODO Ensure wifi connected
    }

    if(config.hasConfig()) {
		if (WiFi.status() != WL_CONNECTED) {
			wifiConnected = false;
			WiFi_connect();
		} else {
			if(!wifiConnected) {
				wifiConnected = true;
				if(debugger) {
					debugger->println("Successfully connected to WiFi!");
					debugger->println(WiFi.localIP());
				}
			}
			if (!config.getMqttHost().isEmpty()) {
				mqtt.loop();
				delay(10); // Needed to preserve power. After adding this, the voltage is super smooth on a HAN powered device
				if(!mqtt.connected() || config.isMqttChanged()) {
					MQTT_connect();
                    initDone = true;
				}
			} else if(mqtt.connected()) {
				mqtt.disconnect();
			}
		}

        if(initDone && config.getUnitBaud() > 0 && config.getUnitId() > 0) {
            if(currentBaud != config.getUnitBaud() || currentId != config.getUnitId()) {
                if(debugger) {
                    debugger->print("Connecting to ventilation unit at baud ");
                    debugger->println(config.getUnitBaud());
                }
                Serial2.begin(config.getUnitBaud());   
                node.begin(config.getUnitId(), Serial2);
                node.preTransmission(preTransmission);
                node.postTransmission(postTransmission);

                currentId = config.getUnitId();
                currentBaud = config.getUnitBaud();
            }

            unsigned long now = millis();
            if(regindex == registers.size()) {
                regindex = 0;
            }
            while(regindex < registers.size()) {
                Register* reg = registers.get(regindex);
                regindex++;
                if(reg->needsUpdate(now)) {
                    if(reg->isCoil()) {
                        if(readCoil(reg)) {
                            reg->setLastUpdated(now);
                        }
                    } else {
                        if(readRegister(reg)) {
                            reg->setLastUpdated(now);
                        }
                    }
                    break;
                }
            }
            yield();
        }
    }

    delay(1); // Allow modem sleep
}

boolean readRegister(Register *reg) {
    if(false) {
        debugger->print("Updating register ");
        debugger->print(reg->getName());
        debugger->print(" starting at ");
        debugger->print(reg->getStart());
        debugger->print(" with length ");
        debugger->println(reg->getLength());
    }

    node.clearResponseBuffer();
    node.clearTransmitBuffer();

    int start = reg->getStart();
    int len = reg->getLength();
    uint8_t result = node.readHoldingRegisters(start, len);
    if(result == node.ku8MBSuccess) {
        for(int i = 0; i< len; i++) {
            int address = start + i + 1;
            String* name = reg->getRegisterName(address);
            if(reg->isReadable(address) && *name != "") {
                short update = (short) node.getResponseBuffer(i);
                if(reg->setValue(address, (int) update)) {
                    String formatted = reg->getFormattedValue(address);
                    sendMqttMessage(name, formatted);
                }
            }
        }
        return true;
    }
    if(debugger) debugger->println(" - failed");
    return false;
}

boolean readCoil(Register *reg) {
    if(false) {
        debugger->print("Updating coil ");
        debugger->println(reg->getName());
    }

    node.clearResponseBuffer();
    node.clearTransmitBuffer();

    int start = reg->getStart();
    int len = reg->getLength();
    uint8_t result = node.readCoils(start, len);
    if(result == node.ku8MBSuccess) {
        for(int i = 0; i < len;) {
            uint16_t raw = node.getResponseBuffer(i/16);
            while(i < len) {
                int address = start + i + 1;
                String* name = reg->getRegisterName(address);
                if(reg->isReadable(address) && *name != "") {
                    int update = raw & 0x1;
                    if(reg->setValue(address, update)) {
                        String formatted = reg->getFormattedValue(address);
                        sendMqttMessage(name, formatted);
                    }
                }
                i++;
                if(i%16 == 0)
                    break;
                raw = raw >> 1;
            }
        }
        return true;
    }
    if(debugger) debugger->println(" - failed");
    return false;
}

void mqttMessageReceived(String &topic, String &payload) {
    if(debugger) {
        debugger->print("Received message for topic ");
        debugger->println(topic);
    }

    if(!topic.startsWith(config.getMqttSubscribeTopic()))
        return;

    String trimmed = topic.substring(config.getMqttSubscribeTopic().length()+1);
    if(debugger) {
        debugger->print(" stripped away base topic gives us: ");
        debugger->println(trimmed);
    }

    int address = 0;
    int current = 0;
    Register* updateReg = NULL;
    for(int i = 0; i < registers.size(); i++) {
        Register* reg = registers.get(i);
        address = reg->getRegisterAddress(trimmed);
        if(address != REG_INVALID) {
            current = reg->getValue(address);
            if(reg->isWriteable(address) && reg->setFormattedValue(address, payload)) {
                updateReg = reg;
            }
            break;
        }
    }
    if(updateReg != NULL) {
        int update = updateReg->getValue(address);
        String* name = updateReg->getRegisterName(address);
        String formatted = updateReg->getFormattedValue(address);
        for(int i = 0; i < 3; i++) {
            uint8_t result = updateReg->isCoil() ? node.writeSingleCoil(address-1, update) : node.writeSingleRegister(address-1, update);
            if(result == node.ku8MBSuccess) {
                sendMqttMessage(name, formatted);
                return;
            }
        }
        if(debugger) debugger->println("Failed to update register / coil");
        updateReg->setValue(address, current);
        formatted = updateReg->getFormattedValue(address);
        sendMqttMessage(name, formatted);
    }
}

void sendMqttMessage(String* name, String &payload) {
    String topic = config.getMqttPublishTopic() + "/" + *name;
    if(debugger) {
        debugger->print("Sending message to ");
        debugger->print(topic);
        debugger->print(" with payload ");
        debugger->println(payload);
    }
    mqtt.publish(topic.c_str(), payload.c_str());
    mqtt.loop();
    yield();
}