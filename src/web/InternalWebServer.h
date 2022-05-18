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

#include "registermanager.h"


#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"


class InternalWebServer {
public:
	typedef std::function<void(String &topic, String &payload)> THandlerFunction;

	InternalWebServer(HwTools*);
  void setup(configuration*, MQTTClient*, RegisterManager *regman, Stream*);
  void loop();
	void setMqttEnabled(bool);
	void setMessageHandler(THandlerFunction);

private:
	/************************************************************************
	// Visitor passed through registry manager to export JSON data on
	// registers.
	/************************************************************************/
	class RegisterVisitor : public RegisterManager::Visitor
	{
			WebServer &m_server;
	    uint32_t   m_visit_cnt; // num MQTT messages sent.
			char       m_json[64];
	public:
	    RegisterVisitor(WebServer &server) 
			: Visitor(0,FLG_VISIT_REG_SINGLE|FLG_VISIT_UPDATED|FLG_VISIT_WRITE_PENDING|FLG_VISIT_UNCHANGED), m_server(server), m_visit_cnt(0) {}

	    uint32_t visitCount() const { return m_visit_cnt; }
	    void     resetVisitCount() { m_visit_cnt=0; }

	    // Implement in your visitor class and return negative to
	    // interrupt traverse.
	    virtual int32_t visit( Register &reg ) { return 0; };
	    virtual int32_t visit( int32_t address, Register &reg );;
	};


private:
	HwTools* hw;
  configuration* config;
	WebConfig webConfig;
	MQTTClient* mqtt;
	bool mqttEnabled = false;
	RegisterManager *m_register_manager_p; //LinkedList<Register*> *registers;
	std::vector<Register*> *registers;
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
