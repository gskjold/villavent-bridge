#include "InternalWebServer.h"
#include "version.h"

#include "root/configuration_html.h"
#include "root/boot_css.h"
#include "root/head_html.h"
#include "root/foot_html.h"
#include "root/upload_html.h"
#include "root/github_svg.h"
#include "root/restart_html.h"
#include "root/restartwait_html.h"
#include "root/notfound_html.h"
#include "root/wifi_html.h"
#include "root/mqtt_html.h"
#include "root/web_html.h"
#include "root/ntp_html.h"
#include "root/index_html.h"
#include "root/setup_html.h"
#include "root/application_js.h"
#include "root/modbus_html.h"

InternalWebServer::InternalWebServer(HwTools* hw) {
	this->hw = hw;
}

void InternalWebServer::setup(configuration* config, MQTTClient* mqtt, Stream* debugger) {
    this->config = config;
    this->debugger = debugger;
	this->mqtt = mqtt;

	char jsuri[32];
	snprintf(jsuri, 32, "/application-%s.js", VERSION);

	server.on(jsuri, HTTP_GET, std::bind(&InternalWebServer::applicationJs, this));
	server.on("/", HTTP_GET, std::bind(&InternalWebServer::indexHtml, this));
	server.on("/", HTTP_POST, std::bind(&InternalWebServer::handleSetup, this));
	server.on("/firmware", HTTP_GET, std::bind(&InternalWebServer::firmwareHtml, this));
	server.on("/firmware", HTTP_POST, std::bind(&InternalWebServer::uploadPost, this), std::bind(&InternalWebServer::firmwareUpload, this));
	server.on("/restart", HTTP_GET, std::bind(&InternalWebServer::restartHtml, this));
	server.on("/restart", HTTP_POST, std::bind(&InternalWebServer::restartPost, this));
	server.on("/restart-wait", HTTP_GET, std::bind(&InternalWebServer::restartWaitHtml, this));
	server.on("/is-alive", HTTP_GET, std::bind(&InternalWebServer::isAliveCheck, this));
	server.on("/github.svg", HTTP_GET, std::bind(&InternalWebServer::githubSvg, this)); 
	server.on("/boot.css", std::bind(&InternalWebServer::bootCss, this));

	server.on("/modbus", std::bind(&InternalWebServer::modbusHtml, this));
	server.on("/wifi", std::bind(&InternalWebServer::wifiHtml, this));
	server.on("/mqtt", std::bind(&InternalWebServer::mqttHtml, this));
	server.on("/web", std::bind(&InternalWebServer::webHtml, this));
	server.on("/ntp", std::bind(&InternalWebServer::ntpHtml, this));

	server.on("/save", std::bind(&InternalWebServer::handleSave, this));
	server.onNotFound(std::bind(&InternalWebServer::notFound, this));

	server.begin(); // Web server start

	print("Web server is ready for config at http://");
	if(WiFi.getMode() == WIFI_AP) {
		print(WiFi.softAPIP());
	} else {
		print(WiFi.localIP());
	}
	println("/");

	config->getWebConfig(webConfig);
	MqttConfig mqttConfig;
	config->getMqttConfig(mqttConfig);
	mqttEnabled = strlen(mqttConfig.host) > 0;
}

void InternalWebServer::setMqttEnabled(bool enabled) {
	mqttEnabled = enabled;
}
void InternalWebServer::loop() {
	server.handleClient();
}
extern "C" {
#include "crypto/base64.h"
}

bool InternalWebServer::checkSecurity(byte level) {
	bool access = WiFi.getMode() == WIFI_AP || webConfig.security < level;
	if(!access && webConfig.security >= level && server.hasHeader("Authorization")) {
		String expectedAuth = String(webConfig.username) + ":" + String(webConfig.password);

		String providedPwd = server.header("Authorization");
		providedPwd.replace("Basic ", "");

		// TODO fiks
		//String expectedBase64 = base64::encode(expectedAuth);
		//access = providedPwd.equals(expectedBase64);
		access = true;
	}

	if(!access) {
		server.sendHeader("WWW-Authenticate", "Basic realm=\"Secure Area\"");
		server.setContentLength(0);
		server.send(401, "text/html", "");
	}
	return access;
}

void InternalWebServer::handleSave() {
	printD("Handling save method from http");
	if(!checkSecurity(1))
		return;

	String temp;

	if(server.hasArg("mbc") && server.arg("mbc") == "true") {
		printD("Received Modbus config");
		SystemConfig sys;
		config->getSystemConfig(sys);
		sys.unitBaud = server.arg("b").toInt();
		sys.unitId = server.arg("i").toInt();
		config->setSystemConfig(sys);
	}

	if(server.hasArg("wc") && server.arg("wc") == "true") {
		printD("Received WiFi config");
		WiFiConfig wifi;
		config->clearWifi(wifi);
		strcpy(wifi.ssid, server.arg("s").c_str());
		strcpy(wifi.psk, server.arg("p").c_str());

		if(server.hasArg("st") && server.arg("st").toInt() == 1) {
			strcpy(wifi.ip, server.arg("i").c_str());
			strcpy(wifi.gateway, server.arg("g").c_str());
			strcpy(wifi.subnet, server.arg("sn").c_str());
			strcpy(wifi.dns1, server.arg("d1").c_str());
			strcpy(wifi.dns2, server.arg("d2").c_str());
		}
		if(server.hasArg("h") && !server.arg("h").isEmpty()) {
			strcpy(wifi.hostname, server.arg("h").c_str());
		}
		config->setWiFiConfig(wifi);
	}

	if(server.hasArg("mqc") && server.arg("mqc") == "true") {
		printD("Received MQTT config");
		MqttConfig mqtt;
		if(server.hasArg("m") && server.arg("m") == "true") {
			strcpy(mqtt.host, server.arg("h").c_str());
			strcpy(mqtt.clientId, server.arg("i").c_str());
			strcpy(mqtt.publishTopic, server.arg("t").c_str());
			strcpy(mqtt.subscribeTopic, server.arg("st").c_str());
			strcpy(mqtt.username, server.arg("u").c_str());
			strcpy(mqtt.password, server.arg("pw").c_str());
			mqtt.ssl = server.arg("s") == "true";

			mqtt.port = server.arg("p").toInt();
			if(mqtt.port == 0) {
				mqtt.port = mqtt.ssl ? 8883 : 1883;
			}
		} else {
			config->clearMqtt(mqtt);
		}
		config->setMqttConfig(mqtt);
	}

	if(server.hasArg("ac") && server.arg("ac") == "true") {
		printD("Received web config");
		webConfig.security = server.arg("as").toInt();
		if(webConfig.security > 0) {
			strcpy(webConfig.username, server.arg("au").c_str());
			strcpy(webConfig.password, server.arg("ap").c_str());
		} else {
			strcpy(webConfig.username, "");
			strcpy(webConfig.password, "");
		}
		config->setWebConfig(webConfig);
	}

	if(server.hasArg("nc") && server.arg("nc") == "true") {
		printD("Received NTP config");
		NtpConfig ntp {
			server.hasArg("n") && server.arg("n") == "true",
			server.hasArg("nd") && server.arg("nd") == "true",
			server.arg("o").toInt() / 10,
			server.arg("so").toInt() / 10
		};
		strcpy(ntp.server, server.arg("ns").c_str());
		config->setNtpConfig(ntp);
	}

	printI("Saving configuration now...");

	// TODO: if (debugger->isActive(RemoteDebug::DEBUG)) config->print(debugger);
	if (config->save()) {
		printI("Successfully saved.");
		if(config->isWifiChanged()) {
			performRestart = true;
            server.sendHeader("Location","/restart-wait");
            server.send(303);
		} else {
			server.sendHeader("Location", String("/"), true);
			server.send (302, "text/plain", "");
		}
	} else {
		printE("Error saving configuration");
		String html = "<html><body><h1>Error saving configuration!</h1></form>";
		server.send(500, "text/html", html);
	}
}

void InternalWebServer::indexHtml() {
	printD("Serving /index.html over http...");

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");

	if(WiFi.getMode() == WIFI_AP) {
		SystemConfig sys;
		config->getSystemConfig(sys);

		WiFiConfig wifi;
		config->clearWifi(wifi);

		String html = String((const __FlashStringHelper*) SETUP_HTML);
		for(int i = 0; i<255; i++) {
			html.replace("${config.boardType" + String(i) + "}", sys.boardType == i ? "selected"  : "");
		}

		html.replace("${config.unitBaud" + String(2400) + "}", sys.unitBaud == 2400 ? "selected"  : "");
		html.replace("${config.unitBaud" + String(4800) + "}", sys.unitBaud == 4800 ? "selected"  : "");
		html.replace("${config.unitBaud" + String(9600) + "}", sys.unitBaud == 9600 ? "selected"  : "");
		html.replace("${config.unitBaud" + String(19200) + "}", sys.unitBaud == 19200 ? "selected"  : "");

		html.replace("${config.unitId}", String(sys.unitId));

		html.replace("${config.wifiSsid}", wifi.ssid);
		html.replace("${config.wifiPassword}", wifi.psk);
		html.replace("${config.wifiStaticIp}", strlen(wifi.ip) > 0 ? "checked" : "");
		html.replace("${config.wifiIp}", wifi.ip);
		html.replace("${config.wifiGw}", wifi.gateway);
		html.replace("${config.wifiSubnet}", wifi.subnet);
		html.replace("${config.wifiDns1}", wifi.dns1);
		html.replace("${config.wifiDns2}", wifi.dns2);
		html.replace("${config.wifiHostname}", wifi.hostname);
		server.send(200, "text/html", html);
	} else {
		if(!checkSecurity(2))
			return;

		String html = String((const __FlashStringHelper*) INDEX_HTML);

		double vcc = hw->getVcc();
		html.replace("{vcc}", vcc > 0 ? String(vcc, 2) : "");

		int rssi = hw->getWifiRssi();
		html.replace("{rssi}", vcc > 0 ? String(rssi) : "");

		html.replace("{cs}", String((uint32_t)(millis64()/1000), 10));

		server.setContentLength(html.length() + HEAD_HTML_LEN + FOOT_HTML_LEN);
		server.send_P(200, "text/html", HEAD_HTML);
		server.sendContent(html);
		server.sendContent_P(FOOT_HTML);
	}
}

void InternalWebServer::bootCss() {
	println("Serving /boot.css over http...");

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(200, "text/css", BOOT_CSS);
}


void InternalWebServer::firmwareHtml() {
	println("Serving /firmware.html over http...");

	if(!checkSecurity(1))
		return;

	uploadHtml("Firmware", "/firmware", "system");
}

void InternalWebServer::firmwareUpload() {
	if(!checkSecurity(1))
		return;

	HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if(!filename.endsWith(".bin")) {
            server.send(500, "text/plain", "500: couldn't create file");
		}
	}
	uploadFile(FILE_FIRMWARE);
	if(upload.status == UPLOAD_FILE_END) {
		performRestart = true;
		server.sendHeader("Location","/restart-wait");
		server.send(303);
	}
}

void InternalWebServer::uploadHtml(const char* label, const char* action, const char* menu) {
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	
	server.setContentLength(UPLOAD_HTML_LEN + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(200, "text/html", HEAD_HTML);
	server.sendContent_P(UPLOAD_HTML);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::uploadFile(const char* path) {
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START){
		if(uploading) {
			println("Upload already in progress");
			String html = "<html><body><h1>Upload already in progress!</h1></form>";
			server.send(500, "text/html", html);
		} else if (!SPIFFS.begin()) {
			println("An Error has occurred while mounting SPIFFS");
			String html = "<html><body><h1>Unable to mount SPIFFS!</h1></form>";
			server.send(500, "text/html", html);
		} else {
			uploading = true;
			print("handleFileUpload file: ");
			println(path);
		    file = SPIFFS.open(path, "w");
            file.write(upload.buf, upload.currentSize);
	    } 
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        if(file)
            file.write(upload.buf, upload.currentSize);
    } else if(upload.status == UPLOAD_FILE_END) {
        if(file) {
			file.flush();
            file.close();
			SPIFFS.end();
			print("handleFileUpload Size: ");
			println(String(upload.totalSize).c_str());
        } else {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}

void InternalWebServer::uploadPost() {
	server.send(200);
}

void InternalWebServer::restartHtml() {
	println("Serving /restart.html over http...");

	if(!checkSecurity(1))
		return;

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");

	server.setContentLength(RESTART_HTML_LEN + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(200, "text/html", HEAD_HTML);
	server.sendContent_P(RESTART_HTML);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::restartPost() {
	if(!checkSecurity(1))
		return;

	println("Setting restart flag and redirecting");
	performRestart = true;
	server.sendHeader("Location","/restart-wait");
	server.send(303);
}

void InternalWebServer::restartWaitHtml() {
	println("Serving /restart-wait.html over http...");

	if(!checkSecurity(1))
		return;

	String html = String((const __FlashStringHelper*) RESTARTWAIT_HTML);

	WiFiConfig wifi;
	config->getWiFiConfig(wifi);

	if(WiFi.getMode() != WIFI_AP) {
		html.replace("boot.css", BOOTSTRAP_URL);
	}
	if(strlen(wifi.ip) == 0 && WiFi.getMode() != WIFI_AP) {
		html.replace("${ip}", WiFi.localIP().toString());
	} else {
		html.replace("${ip}", wifi.ip);
	}
	html.replace("${hostname}", wifi.hostname);

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");

	server.setContentLength(html.length());
	server.send(200, "text/html", html);

	yield();
	if(performRestart) {
		SPIFFS.end();
		println("Rebooting");
		delay(1000);
		ESP.restart();
		performRestart = false;
	}
}

void InternalWebServer::isAliveCheck() {
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(200);
}

void InternalWebServer::githubSvg() {
	println("Serving /github.svg over http...");

	server.sendHeader("Cache-Control", "public, max-age=3600");
	server.send_P(200, "image/svg+xml", GITHUB_SVG);
}

void InternalWebServer::notFound() {
	server.sendHeader("Cache-Control", "public, max-age=3600");
	
	server.setContentLength(NOTFOUND_HTML_LEN + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(404, "text/html", HEAD_HTML);
	server.sendContent_P(NOTFOUND_HTML);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::modbusHtml() {
	printD("Serving /modbus.html over http...");

	if(!checkSecurity(1))
		return;

	String html = String((const __FlashStringHelper*) MODBUS_HTML);

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");

	SystemConfig sys;
	config->getSystemConfig(sys);

	html.replace("{b" + String(2400) + "}", sys.unitBaud == 2400 ? "selected"  : "");
	html.replace("{b" + String(4800) + "}", sys.unitBaud == 4800 ? "selected"  : "");
	html.replace("{b" + String(9600) + "}", sys.unitBaud == 9600 ? "selected"  : "");
	html.replace("{b" + String(19200) + "}", sys.unitBaud == 19200 ? "selected"  : "");

	html.replace("{i}", String(sys.unitId));

	server.setContentLength(html.length() + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(200, "text/html", HEAD_HTML);
	server.sendContent(html);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::wifiHtml() {
	printD("Serving /wifi.html over http...");

	if(!checkSecurity(1))
		return;

	String html = String((const __FlashStringHelper*) WIFI_HTML);

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");

	WiFiConfig wifi;
	config->getWiFiConfig(wifi);

	html.replace("{s}", wifi.ssid);
	html.replace("{p}", wifi.psk);
	html.replace("{st}", strlen(wifi.ip) > 0 ? "checked" : "");
	html.replace("{i}", wifi.ip);
	html.replace("{g}", wifi.gateway);
	html.replace("{sn}", wifi.subnet);
	html.replace("{d1}", wifi.dns1);
	html.replace("{d2}", wifi.dns2);
	html.replace("{h}", wifi.hostname);
	html.replace("{m}", wifi.mdns ? "checked" : "");

	server.setContentLength(html.length() + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(200, "text/html", HEAD_HTML);
	server.sendContent(html);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::mqttHtml() {
	printD("Serving /mqtt.html over http...");

	if(!checkSecurity(1))
		return;

	String html = String((const __FlashStringHelper*) MQTT_HTML);

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");

	MqttConfig mqtt;
	config->getMqttConfig(mqtt);

	html.replace("{m}", strlen(mqtt.host) == 0 ? "" : "checked");
	html.replace("{h}", mqtt.host);
	if(mqtt.port > 0) {
		html.replace("{p}", String(mqtt.port));
	} else {
		html.replace("{p}", String(1883));
	}
	html.replace("{i}", mqtt.clientId);
	html.replace("{t}", mqtt.publishTopic);
	html.replace("{st}", mqtt.subscribeTopic);
	html.replace("{u}", mqtt.username);
	html.replace("{pw}", mqtt.password);

	html.replace("{s}", mqtt.ssl ? "checked" : "");

	if(SPIFFS.begin()) {
		html.replace("{dcu}", SPIFFS.exists(FILE_MQTT_CA) ? "none" : "");
		html.replace("{dcf}", SPIFFS.exists(FILE_MQTT_CA) ? "" : "none");
		html.replace("{deu}", SPIFFS.exists(FILE_MQTT_CERT) ? "none" : "");
		html.replace("{def}", SPIFFS.exists(FILE_MQTT_CERT) ? "" : "none");
		html.replace("{dku}", SPIFFS.exists(FILE_MQTT_KEY) ? "none" : "");
		html.replace("{dkf}", SPIFFS.exists(FILE_MQTT_KEY) ? "" : "none");
		SPIFFS.end();
	} else {
		html.replace("{dcu}", "");
		html.replace("{dcf}", "none");
		html.replace("{deu}", "");
		html.replace("{def}", "none");
		html.replace("{dku}", "");
		html.replace("{dkf}", "none");
	}

	server.setContentLength(html.length() + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(200, "text/html", HEAD_HTML);
	server.sendContent(html);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::webHtml() {
	printD("Serving /web.html over http...");

	if(!checkSecurity(1))
		return;

	String html = String((const __FlashStringHelper*) WEB_HTML);

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");

	html.replace("{as}", String(webConfig.security));
	for(int i = 0; i<3; i++) {
		html.replace("{as" + String(i) + "}", webConfig.security == i ? "selected"  : "");
	}
	html.replace("{au}", webConfig.username);
	html.replace("{ap}", webConfig.password);

	server.setContentLength(html.length() + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(200, "text/html", HEAD_HTML);
	server.sendContent(html);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::ntpHtml() {
	printD("Serving /ntp.html over http...");

	if(!checkSecurity(1))
		return;

	String html = String((const __FlashStringHelper*) NTP_HTML);

	NtpConfig ntp;
	config->getNtpConfig(ntp);

	html.replace("{n}", ntp.enable ? "checked" : "");

	for(int i = (3600*-13); i<(3600*15); i+=3600) {
		html.replace("{o" + String(i) + "}", ntp.offset * 10 == i ? "selected"  : "");
	}

	for(int i = 0; i<(3600*3); i+=3600) {
		html.replace("{so" + String(i) + "}", ntp.summerOffset * 10 == i ? "selected"  : "");
	}

	html.replace("{ns}", ntp.server);
	html.replace("{nd}", ntp.dhcp ? "checked" : "");

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");

	server.setContentLength(html.length() + HEAD_HTML_LEN + FOOT_HTML_LEN);
	server.send_P(200, "text/html", HEAD_HTML);
	server.sendContent(html);
	server.sendContent_P(FOOT_HTML);
}

void InternalWebServer::handleSetup() {
	printD("Handling setup method from http");

	if(!server.hasArg("wifiSsid") || server.arg("wifiSsid").isEmpty() || !server.hasArg("wifiPassword") || server.arg("wifiPassword").isEmpty()) {
		server.sendHeader("Location", String("/"), true);
		server.send (302, "text/plain", "");
	} else {
		SystemConfig sys { 
			server.arg("board").toInt(), 
			server.arg("unitBaud").toInt(), 
			server.arg("unitId").toInt() 
		};

		config->clear();

		switch(sys.boardType) {
			case 100: // ESP32
			/*
				gpioConfig->apPin = 0;
				gpioConfig->ledPin = 2;
				gpioConfig->ledInverted = false;
			*/
				break;
		}

		WiFiConfig wifi;
		config->clearWifi(wifi);

		strcpy(wifi.ssid, server.arg("wifiSsid").c_str());
		strcpy(wifi.psk, server.arg("wifiPassword").c_str());

		if(server.hasArg("wifiIpType") && server.arg("wifiIpType").toInt() == 1) {
			strcpy(wifi.ip, server.arg("wifiIp").c_str());
			strcpy(wifi.gateway, server.arg("wifiGw").c_str());
			strcpy(wifi.subnet, server.arg("wifiSubnet").c_str());
			strcpy(wifi.dns1, server.arg("wifiDns1").c_str());
		}
		if(server.hasArg("wifiHostname") && !server.arg("wifiHostname").isEmpty()) {
			strcpy(wifi.hostname, server.arg("wifiHostname").c_str());
			wifi.mdns = true;
		} else {
			wifi.mdns = false;
		}
		
		MqttConfig mqttConfig;
		config->clearMqtt(mqttConfig);
		
		config->clearAuth(webConfig);

		NtpConfig ntp;
		config->clearNtp(ntp);

		bool success = true;
		if(!config->setSystemConfig(sys)) {
			printD("Unable to set system config");
			success = false;
		}
		if(!config->setWiFiConfig(wifi)) {
			printD("Unable to set WiFi config");
			success = false;
		}
		if(!config->setMqttConfig(mqttConfig)) {
			printD("Unable to set MQTT config");
			success = false;
		}
		if(!config->setWebConfig(webConfig)) {
			printD("Unable to set web config");
			success = false;
		}
		if(!config->setNtpConfig(ntp)) {
			printD("Unable to set NTP config");
			success = false;
		}

		if(success && config->save()) {
			performRestart = true;
			server.sendHeader("Location","/restart-wait");
			server.send(303);
		} else {
			printE("Error saving configuration");
			String html = "<html><body><h1>Error saving configuration!</h1></form>";
			server.send(500, "text/html", html);
		}
	}
}

void InternalWebServer::applicationJs() {
	printD("Serving /application.js over http...");

	server.sendHeader("Cache-Control", "public, max-age=3600");
	server.send_P(200, "application/javascript", APPLICATION_JS);
}

size_t InternalWebServer::printD(const char* text)
{
	if (debugger) debugger->printf("[DEBUG  ] %s\n", text);
}

size_t InternalWebServer::printI(const char* text)
{
	if (debugger) debugger->printf("[INFO   ] %s\n", text);
}

size_t InternalWebServer::printW(const char* text)
{
	if (debugger) debugger->printf("[WARNING] %s\n", text);
}

size_t InternalWebServer::printE(const char* text)
{
	if (debugger) debugger->printf("[ERROR  ] %s\n", text);
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
