#ifndef _INTERNALWEBSERVER_h
#define _INTERNALWEBSERVER_h

#include "configuration.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <WiFi.h>
#include <WebServer.h>

class InternalWebServer {
public:
    void setup(configuration* config, Stream* debugger);
    void loop();

private:
    configuration* config;
	Stream* debugger;

	WebServer server;

	void configurationHtml();
	void bootCss();

	void handleSave();

   	size_t print(const char* text);
	size_t println(const char* text);
	size_t print(const Printable& data);
	size_t println(const Printable& data);
};

#endif
