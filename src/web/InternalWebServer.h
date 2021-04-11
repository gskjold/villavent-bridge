#ifndef _INTERNALWEBSERVER_h
#define _INTERNALWEBSERVER_h

#define BOOTSTRAP_URL "https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.4.1/css/bootstrap.min.css"
#define FILE_FIRMWARE "/firmware.bin"
#define FILE_MQTT_CA "/mqtt-ca.pem"
#define FILE_MQTT_CERT "/mqtt-cert.pem"
#define FILE_MQTT_KEY "/mqtt-key.pem"

#include "configuration.h"
#include "Uptime.h"
#include "HwTools.h"
#include "register/register.h"

#include "Arduino.h"
#include <MQTT.h>
#include <LinkedList.h>

#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"


class InternalWebServer {
public:
	typedef std::function<void(String &topic, String &payload)> THandlerFunction;

	InternalWebServer(HwTools*);
    void setup(configuration*, MQTTClient*, LinkedList<Register*>*, Stream*);
    void loop();
	void setMqttEnabled(bool);
	void setMessageHandler(THandlerFunction);

private:
	HwTools* hw;
    configuration* config;
	WebConfig webConfig;
	MQTTClient* mqtt;
	bool mqttEnabled = false;
	LinkedList<Register*>* registers;
	THandlerFunction fn;
	Stream* debugger;

	bool uploading = false;
	File file;
	bool performRestart = false;

	WebServer server;

	bool checkSecurity(byte level);

	void indexHtml();
	void bootCss();
	void firmwareHtml();
	void firmwareUpload();
	void uploadHtml(const char* label, const char* action, const char* menu);
	void uploadFile(const char* path);
	void uploadPost();
	void restartHtml();
	void restartPost();
	void restartWaitHtml();
	void isAliveCheck();
	void githubSvg();
	void applicationJs();
	void notFound();

	void dataJson();
	void writeJson();

	void modbusHtml();
	void wifiHtml();
	void mqttHtml();
	void webHtml();
	void ntpHtml();

	void handleSave();
	void handleSetup();

   	size_t print(const char* text);
	size_t println(const char* text);
	size_t print(const Printable& data);
	size_t println(const Printable& data);

   	size_t printD(const char* text);
   	size_t printI(const char* text);
   	size_t printW(const char* text);
   	size_t printE(const char* text);
};

#endif
