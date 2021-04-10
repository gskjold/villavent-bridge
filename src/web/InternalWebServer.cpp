#include "InternalWebServer.h"

#include "root/configuration_html.h"
#include "root/boot_css.h"

WebServer server(80);

void InternalWebServer::setup(configuration* config, Stream* debugger) {
    this->config = config;
    this->debugger = debugger;

	server.on("/", std::bind(&InternalWebServer::configurationHtml, this));
	server.on("/boot.css", std::bind(&InternalWebServer::bootCss, this));
	server.on("/save", std::bind(&InternalWebServer::handleSave, this));

	server.begin(); // Web server start

	print("Web server is ready for config at http://");
	if(WiFi.getMode() == WIFI_AP) {
		print(WiFi.softAPIP());
	} else {
		print(WiFi.localIP());
	}
	println("/");
}

void InternalWebServer::loop() {
	server.handleClient();
}

void InternalWebServer::configurationHtml() {
	println("Serving /configuration.html over http...");

	String html = String((const __FlashStringHelper*) CONFIGURATION_HTML);
//	html.replace("${version}", VERSION);

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");

	if(config->hasConfig()) {
		html.replace("${config.wifiSsid}", config->getWifiSsid());
		html.replace("${config.wifiPassword}", config->getWifiPassword());
		html.replace("${config.wifiIp}", config->getWifiIp());
		html.replace("${config.wifiGw}", config->getWifiGw());
		html.replace("${config.wifiSubnet}", config->getWifiSubnet());
		html.replace("${config.mqtt}", config->getMqttHost() == "" ? "" : "checked");
		html.replace("${config.mqttHost}", config->getMqttHost());
		html.replace("${config.mqttPort}", String(config->getMqttPort()));
		html.replace("${config.mqttClientId}", config->getMqttClientId());
		html.replace("${config.mqttPublishTopic}", config->getMqttPublishTopic());
		html.replace("${config.mqttSubscribeTopic}", config->getMqttSubscribeTopic());
		html.replace("${config.mqttUser}", config->getMqttUser());
		html.replace("${config.mqttPassword}", config->getMqttPassword());
		html.replace("${config.authSecurity}", String(config->getAuthSecurity()));
		for(int i = 0; i<3; i++) {
			html.replace("${config.authSecurity" + String(i) + "}", config->getAuthSecurity() == i ? "selected"  : "");
		}
		html.replace("${config.authUser}", config->getAuthUser());
		html.replace("${config.authPassword}", config->getAuthPassword());

		for(int i = 2400; i<=19200; i+=2400) {
			html.replace("${config.unitBaud" + String(i) + "}", config->getUnitBaud() == i ? "selected"  : "");
		}
		html.replace("${config.unitId}", String(config->getUnitId()));
	} else {
		html.replace("${config.wifiSsid}", "");
		html.replace("${config.wifiPassword}", "");
		html.replace("${config.wifiIp}", "");
		html.replace("${config.wifiGw}", "");
		html.replace("${config.wifiSubnet}", "");
		html.replace("${config.mqtt}", "");
		html.replace("${config.mqttHost}", "");
		html.replace("${config.mqttPort}", "1883");
		html.replace("${config.mqttClientId}", "");
		html.replace("${config.mqttPublishTopic}", "");
		html.replace("${config.mqttSubscribeTopic}", "");
		html.replace("${config.mqttUser}", "");
		html.replace("${config.mqttPassword}", "");
		html.replace("${config.authSecurity}", "");
		for(int i = 0; i<3; i++) {
			html.replace("${config.authSecurity" + String(i) + "}", i == 0 ? "selected"  : "");
		}
		html.replace("${config.authUser}", "");
		html.replace("${config.authPassword}", "");

		for(int i = 2400; i<=19200; i+=2400) {
			html.replace("${config.unitBaud" + String(i) + "}", i == 0 ? "selected"  : "");
		}
		html.replace("${config.unitId}", "");
	}
	server.send(200, "text/html", html);
}

void InternalWebServer::handleSave() {
	String temp;

	config->setWifiSsid(server.arg("wifiSsid"));
	config->setWifiPassword(server.arg("wifiPassword"));
	config->setWifiIp(server.arg("wifiIp"));
	config->setWifiGw(server.arg("wifiGw"));
	config->setWifiSubnet(server.arg("wifiSubnet"));

	if(server.hasArg("mqtt") && server.arg("mqtt") == "true") {
		config->setMqttHost(server.arg("mqttHost"));
		config->setMqttPort(server.arg("mqttPort").toInt());
		config->setMqttClientId(server.arg("mqttClientId"));
		config->setMqttPublishTopic(server.arg("mqttPublishTopic"));
		config->setMqttSubscribeTopic(server.arg("mqttSubscribeTopic"));
		config->setMqttUser(server.arg("mqttUser"));
		config->setMqttPassword(server.arg("mqttPassword"));
		config->setAuthUser(server.arg("authUser"));
		config->setAuthPassword(server.arg("authPassword"));
	} else {
		config->clearMqtt();
	}

	config->setAuthSecurity((byte)server.arg("authSecurity").toInt());
	if(config->getAuthSecurity() > 0) {
		config->setAuthUser(server.arg("authUser"));
		config->setAuthPassword(server.arg("authPassword"));
	} else {
		config->clearAuth();
	}
	

	config->setUnitBaud(server.arg("unitBaud").toInt());
	config->setUnitId(server.arg("unitId").toInt());

	println("Saving configuration now...");

	if (config->save()) {
		println("Successfully saved.");
		String html = "<html><body><h1>Successfully Saved!</h1><a href=\"/\">Go to index</a></form>";
		server.send(200, "text/html", html);
		yield();
		if(config->isWifiChanged()) {
			println("Wifi config changed, rebooting");
			delay(1000);
			ESP.restart();
		}
	} else {
		println("Error saving configuration");
		String html = "<html><body><h1>Error saving configuration!</h1></form>";
		server.send(500, "text/html", html);
	}
}

void InternalWebServer::bootCss() {
	println("Serving /boot.css over http...");

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(200, "text/css", BOOT_CSS);
}


size_t InternalWebServer::print(const char* text)
{
	if (debugger) debugger->print(text);
}
size_t InternalWebServer::println(const char* text)
{
	if (debugger) debugger->println(text);
}
size_t InternalWebServer::print(const Printable& data)
{
	if (debugger) debugger->print(data);
}
size_t InternalWebServer::println(const Printable& data)
{
	if (debugger) debugger->println(data);
}
