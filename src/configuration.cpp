#include "configuration.h"

bool configuration::getSystemConfig(SystemConfig& config) {
	if(hasConfig()) {
		EEPROM.begin(EEPROM_SIZE);
		EEPROM.get(CONFIG_SYSTEM_START, config);
		EEPROM.end();
		systemChanged = true;
		return true;
	} else {
		return false;
	}
}

bool configuration::setSystemConfig(SystemConfig& config) {
	SystemConfig existing;
	if(getSystemConfig(existing)) {
		systemChanged |= config.boardType != existing.boardType;
		systemChanged |= config.unitBaud != existing.unitBaud;
		systemChanged |= config.unitId != existing.unitId;
	} else {
		systemChanged = true;
	}
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(CONFIG_SYSTEM_START, config);
	bool ret = EEPROM.commit();
	EEPROM.end();
	return ret;
}

bool configuration::isSystemChanged() {
	return systemChanged;
}

void configuration::ackSystemChange() {
	systemChanged = false;
}

bool configuration::getWiFiConfig(WiFiConfig& config) {
	if(hasConfig()) {
		EEPROM.begin(EEPROM_SIZE);
		EEPROM.get(CONFIG_WIFI_START, config);
		EEPROM.end();
		return true;
	} else {
		clearWifi(config);
		return false;
	}
}

bool configuration::setWiFiConfig(WiFiConfig& config) {
	WiFiConfig existing;
	if(getWiFiConfig(existing)) {
		wifiChanged |= strcmp(config.ssid, existing.ssid) != 0;
		wifiChanged |= strcmp(config.psk, existing.psk) != 0;
		wifiChanged |= strcmp(config.ip, existing.ip) != 0;
		wifiChanged |= strcmp(config.gateway, existing.gateway) != 0;
		wifiChanged |= strcmp(config.subnet, existing.subnet) != 0;
		wifiChanged |= strcmp(config.dns1, existing.dns1) != 0;
		wifiChanged |= strcmp(config.dns2, existing.dns2) != 0;
		wifiChanged |= strcmp(config.hostname, existing.hostname) != 0;
	} else {
		wifiChanged = true;
	}
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(CONFIG_WIFI_START, config);
	bool ret = EEPROM.commit();
	EEPROM.end();
	return ret;
}

void configuration::clearWifi(WiFiConfig& config) {
	strcpy(config.ssid, "");
	strcpy(config.psk, "");
	clearWifiIp(config);

	uint16_t chipId;
	#if defined(ESP32)
		chipId = ESP.getEfuseMac();
	#else
		chipId = ESP.getChipId();
	#endif
	strcpy(config.hostname, (String("villavent-") + String(chipId, HEX)).c_str());
	config.mdns = true;
}

void configuration::clearWifiIp(WiFiConfig& config) {
	strcpy(config.ip, "");
	strcpy(config.gateway, "");
	strcpy(config.subnet, "");
	strcpy(config.dns1, "");
	strcpy(config.dns2, "");
}

bool configuration::isWifiChanged() {
	return wifiChanged;
}

void configuration::ackWifiChange() {
	wifiChanged = false;
}

bool configuration::getMqttConfig(MqttConfig& config) {
	if(hasConfig()) {
		EEPROM.begin(EEPROM_SIZE);
		EEPROM.get(CONFIG_MQTT_START, config);
		EEPROM.end();
		return true;
	} else {
		clearMqtt(config);
		return false;
	}
}

bool configuration::setMqttConfig(MqttConfig& config) {
	MqttConfig existing;
	if(getMqttConfig(existing)) {
		mqttChanged |= strcmp(config.host, existing.host) != 0;
		mqttChanged |= config.port != existing.port;
		mqttChanged |= strcmp(config.clientId, existing.clientId) != 0;
		mqttChanged |= strcmp(config.publishTopic, existing.publishTopic) != 0;
		mqttChanged |= strcmp(config.subscribeTopic, existing.subscribeTopic) != 0;
		mqttChanged |= strcmp(config.username, existing.username) != 0;
		mqttChanged |= strcmp(config.password, existing.password) != 0;
		mqttChanged |= config.ssl != existing.ssl;
	} else {
		mqttChanged = true;
	}
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(CONFIG_MQTT_START, config);
	bool ret = EEPROM.commit();
	EEPROM.end();
	return ret;
}

void configuration::clearMqtt(MqttConfig& config) {
	strcpy(config.host, "");
	config.port = 1883;
	strcpy(config.clientId, "");
	strcpy(config.publishTopic, "");
	strcpy(config.subscribeTopic, "");
	strcpy(config.username, "");
	strcpy(config.password, "");
	config.ssl = false;
}

void configuration::setMqttChanged() {
	mqttChanged = true;
}

bool configuration::isMqttChanged() {
	return mqttChanged;
}

void configuration::ackMqttChange() {
	mqttChanged = false;
}

bool configuration::getWebConfig(WebConfig& config) {
	if(hasConfig()) {
		EEPROM.begin(EEPROM_SIZE);
		EEPROM.get(CONFIG_WEB_START, config);
		EEPROM.end();
		return true;
	} else {
		clearAuth(config);
		return false;
	}
}

bool configuration::setWebConfig(WebConfig& config) {
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(CONFIG_WEB_START, config);
	bool ret = EEPROM.commit();
	EEPROM.end();
	return ret;
}

void configuration::clearAuth(WebConfig& config) {
	config.security = 0;
	strcpy(config.username, "");
	strcpy(config.password, "");
}

bool configuration::getNtpConfig(NtpConfig& config) {
	if(hasConfig()) {
		EEPROM.begin(EEPROM_SIZE);
		EEPROM.get(CONFIG_NTP_START, config);
		EEPROM.end();
		return true;
	} else {
		clearNtp(config);
		return false;
	}
}

bool configuration::setNtpConfig(NtpConfig& config) {
	NtpConfig existing;
	if(getNtpConfig(existing)) {
		if(config.enable != existing.enable) {
			if(!existing.enable) {
				wifiChanged = true;
			} else {
				ntpChanged = true;
			}
		}
		ntpChanged |= config.dhcp != existing.dhcp;
		ntpChanged |= config.offset != existing.offset;
		ntpChanged |= config.summerOffset != existing.summerOffset;
		ntpChanged |= strcmp(config.server, existing.server) != 0;
	} else {
		ntpChanged = true;
	}
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(CONFIG_NTP_START, config);
	bool ret = EEPROM.commit();
	EEPROM.end();
	return ret;
}

bool configuration::isNtpChanged() {
	return ntpChanged;
}

void configuration::ackNtpChange() {
	ntpChanged = false;
}

void configuration::clearNtp(NtpConfig& config) {
	config.enable = true;
	config.dhcp = true;
	config.offset = 360;
	config.summerOffset = 360;
	strcpy(config.server, "pool.ntp.org");
}


bool configuration::hasConfig() {
	if(configVersion == 0) {
		EEPROM.begin(EEPROM_SIZE);
		configVersion = EEPROM.read(EEPROM_CONFIG_ADDRESS);
		EEPROM.end();
	}
	switch(configVersion) {
		case 42:
			configVersion = -1; // Prevent loop
			if(loadConfig42()) {
				configVersion = EEPROM_CHECK_SUM;
				return true;
			} else {
				return false;
			}
			break;
		case EEPROM_CHECK_SUM:
			return true;
		default:
			configVersion = -1;
			return false;
	}
}

bool configuration::loadConfig42() {
	EEPROM.begin(512);
	char* ssid;
	char* psk;
	char* ip;
	char* gw;
	char* subnet;

	int cs = EEPROM.read(0);
	Serial.printf("Config version: %d\n", cs);
	int address = 1;

	address += readString(address, &ssid);
	address += readString(address, &psk);
	address += readString(address, &ip);
	address += readString(address, &gw);
	address += readString(address, &subnet);

	Serial.printf("SSID: %s\n", ssid);
	Serial.printf("PSK: %s\n", psk);
	Serial.printf("IP: %s\n", ip);
	Serial.printf("GW: %s\n", gw);
	Serial.printf("Subnet: %s\n", subnet);

	WiFiConfig wifi;
	clearWifi(wifi);
	strcpy(wifi.ssid, ssid);
	strcpy(wifi.psk, psk);
	if(strlen(ip) > 0) {
		strcpy(wifi.ip, ip);
		strcpy(wifi.gateway, gw);
		strcpy(wifi.subnet, subnet);
		strcpy(wifi.dns1, gw);
	}

	MqttConfig mqtt;
	clearMqtt(mqtt);

	char* host;
	int port;
	char* clientId;
	char* pub;
	char* sub;
	char* user;
	char* pass;

	bool mqtt_en = false;
	address += readBool(address, &mqtt_en);
	if(mqtt_en) {
		address += readString(address, &host);
		address += readInt(address, &port);
		address += readString(address, &clientId);
		address += readString(address, &pub);
		address += readString(address, &sub);

		Serial.printf("Host: %s\n", host);
		Serial.printf("Port: %d\n", port);
		Serial.printf("Client ID: %s\n", clientId);
		Serial.printf("Pub: %s\n", pub);
		Serial.printf("Sub: %s\n", sub);

		strcpy(mqtt.host, host);
		mqtt.port = port;
		strcpy(mqtt.clientId, clientId);
		strcpy(mqtt.publishTopic, pub);
		strcpy(mqtt.subscribeTopic, sub);

		bool secure = false;
		address += readBool(address, &secure);
		if (secure) {
			address += readString(address, &user);
			address += readString(address, &pass);

			strcpy(mqtt.username, user);
			strcpy(mqtt.password, pass);
		}
	}

	WebConfig web;

	char* username;
	char* password;

	uint8_t authSecurity;
	address += readByte(address, &authSecurity);
	if (authSecurity > 0) {
		address += readString(address, &username);
		address += readString(address, &password);
		strcpy(web.username, username);
		strcpy(web.password, password);
	} else {
		strcpy(web.username, "");
		strcpy(web.password, "");
	}
	web.security = authSecurity;

	int baud;
	int unitId;
	address += readInt(address, &baud);
	address += readInt(address, &unitId);
	EEPROM.end();
	Serial.println("End");

	SystemConfig sys;
	sys.boardType = 100;
	sys.unitBaud = baud;
	sys.unitId = unitId;

	NtpConfig ntp;
	clearNtp(ntp);

	Serial.println("Store");
	Serial.flush();
	delay(100);
	setSystemConfig(sys);
	setWiFiConfig(wifi);
	setMqttConfig(mqtt);
	setWebConfig(web);
	setNtpConfig(ntp);
	Serial.println("Save");
	Serial.flush();
	delay(100);
	return save();
}

bool configuration::save() {
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(EEPROM_CONFIG_ADDRESS, EEPROM_CHECK_SUM);
	bool success = EEPROM.commit();
	EEPROM.end();

	configVersion = EEPROM_CHECK_SUM;
	return success;
}

void configuration::clear() {
	EEPROM.begin(EEPROM_SIZE);

	WiFiConfig wifi;
	clearWifi(wifi);
	EEPROM.put(CONFIG_WIFI_START, wifi);

	MqttConfig mqtt;
	clearMqtt(mqtt);
	EEPROM.put(CONFIG_MQTT_START, mqtt);

	WebConfig web;
	clearAuth(web);
	EEPROM.put(CONFIG_WEB_START, web);

	NtpConfig ntp;
	clearNtp(ntp);
	EEPROM.put(CONFIG_NTP_START, ntp);

	EEPROM.put(EEPROM_CONFIG_ADDRESS, -1);
	EEPROM.commit();
	EEPROM.end();
}

int configuration::readString(int pAddress, char* pString[]) {
	int address = 0;
	byte length = EEPROM.read(pAddress + address);
	address++;

	char* buffer = new char[length];
	for (int i = 0; i<length; i++)
	{
		buffer[i] = EEPROM.read(pAddress + address++);
	}
	*pString = buffer;
	return address;
}

int configuration::readInt(int address, int *value) {
	int lower = EEPROM.read(address);
	int higher = EEPROM.read(address + 1);
	*value = lower + (higher << 8);
	return 2;
}

int configuration::readBool(int address, bool *value) {
	byte y = EEPROM.read(address);
	*value = (bool)y;
	return 1;
}

int configuration::readByte(int address, byte *value) {
	*value = EEPROM.read(address);
	return 1;
}

template <class T> int configuration::readAnything(int ee, T& value) {
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}
