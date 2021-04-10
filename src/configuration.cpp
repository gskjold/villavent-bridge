#include "configuration.h"

String configuration::getWifiSsid() {
	return wifiSsid;
}

void configuration::setWifiSsid(String wifiSsid) {
	wifiChanged |= this->wifiSsid != wifiSsid;
	this->wifiSsid = String(wifiSsid);
}

String configuration::getWifiPassword() {
	return wifiPassword;
}

void configuration::setWifiPassword(String wifiPassword) {
	wifiChanged |= this->wifiPassword != wifiPassword;
	this->wifiPassword = String(wifiPassword);
}

String configuration::getWifiIp() {
	return wifiIp;
}

void configuration::setWifiIp(String wifiIp) {
	wifiChanged |= this->wifiIp != wifiIp;
	this->wifiIp = String(wifiIp);
}

String configuration::getWifiGw() {
	return wifiGw;
}

void configuration::setWifiGw(String wifiGw) {
	wifiChanged |= this->wifiGw != wifiGw;
	this->wifiGw = String(wifiGw);
}

String configuration::getWifiSubnet() {
	return wifiSubnet;
}

void configuration::setWifiSubnet(String wifiSubnet) {
	wifiChanged |= this->wifiSubnet != wifiSubnet;
	this->wifiSubnet = String(wifiSubnet);
}

bool configuration::isWifiChanged() {
	return wifiChanged;
}

void configuration::ackWifiChange() {
	wifiChanged = false;
}


String configuration::getMqttHost() {
	return mqttHost;
}

void configuration::setMqttHost(String mqttHost) {
	mqttChanged |= this->mqttHost != mqttHost;
	this->mqttHost = String(mqttHost);
}

int configuration::getMqttPort() {
	return mqttPort;
}

void configuration::setMqttPort(int mqttPort) {
	mqttChanged |= this->mqttPort != mqttPort;
	this->mqttPort = mqttPort;
}

String configuration::getMqttClientId() {
	return mqttClientId;
}

void configuration::setMqttClientId(String mqttClientId) {
	mqttChanged |= this->mqttClientId != mqttClientId;
	this->mqttClientId = String(mqttClientId);
}

String configuration::getMqttPublishTopic() {
	return mqttPublishTopic;
}

void configuration::setMqttPublishTopic(String mqttPublishTopic) {
	mqttChanged |= this->mqttPublishTopic != mqttPublishTopic;
	this->mqttPublishTopic = String(mqttPublishTopic);
}

String configuration::getMqttSubscribeTopic() {
	return mqttSubscribeTopic;
}

void configuration::setMqttSubscribeTopic(String mqttSubscribeTopic) {
	mqttChanged |= this->mqttSubscribeTopic != mqttSubscribeTopic;
	this->mqttSubscribeTopic = String(mqttSubscribeTopic);
}

String configuration::getMqttUser() {
	return mqttUser;
}

void configuration::setMqttUser(String mqttUser) {
	mqttChanged |= this->mqttUser != mqttUser;
	this->mqttUser = String(mqttUser);
}

String configuration::getMqttPassword() {
	return mqttPassword;
}

void configuration::setMqttPassword(String mqttPassword) {
	mqttChanged |= this->mqttPassword != mqttPassword;
	this->mqttPassword = String(mqttPassword);
}

void configuration::clearMqtt() {
	setMqttHost("");
	setMqttPort(1883);
	setMqttClientId("");
	setMqttPublishTopic("");
	setMqttSubscribeTopic("");
	setMqttUser("");
	setMqttPassword("");
}

bool configuration::isMqttChanged() {
	return mqttChanged;
}

void configuration::ackMqttChange() {
	mqttChanged = false;
}


byte configuration::getAuthSecurity() {
	return authSecurity;
}

void configuration::setAuthSecurity(byte authSecurity) {
	this->authSecurity = authSecurity;
}

String configuration::getAuthUser() {
	return authUser;
}

void configuration::setAuthUser(String authUser) {
	this->authUser = String(authUser);
}

String configuration::getAuthPassword() {
	return authPassword;
}

void configuration::setAuthPassword(String authPassword) {
	this->authPassword = String(authPassword);
}

void configuration::clearAuth() {
	setAuthSecurity(0);
	setAuthUser("");
	setAuthPassword("");
}


int configuration::getUnitBaud() {
	return unitBaud;
}

void configuration::setUnitBaud(int unitBaud) {
	this->unitBaud = unitBaud;
}

int configuration::getUnitId() {
	return unitId;
}

void configuration::setUnitId(int unitId) {
	this->unitId = unitId;
}


bool configuration::hasConfig() 
{
	if(configVersion == 0) {
		EEPROM.begin(EEPROM_SIZE);
		configVersion = EEPROM.read(EEPROM_CONFIG_ADDRESS);
		Serial.print("Config version: ");
		Serial.println(configVersion);
		EEPROM.end();
	}
	return configVersion == EEPROM_CHECK_SUM;
}

bool configuration::load() {
	int address = EEPROM_CONFIG_ADDRESS;
	bool success = false;

	EEPROM.begin(EEPROM_SIZE);
	int cs = EEPROM.read(address);
	if (cs == EEPROM_CHECK_SUM)
	{
		char* temp;
		address++;

		address += readString(address, &temp);
		setWifiSsid(temp);
		address += readString(address, &temp);
		setWifiPassword(temp);
		address += readString(address, &temp);
		setWifiIp(temp);
		address += readString(address, &temp);
		setWifiGw(temp);
		address += readString(address, &temp);
		setWifiSubnet(temp);

		bool mqtt = false;
		address += readBool(address, &mqtt);
		if(mqtt) {
			address += readString(address, &temp);
			setMqttHost(temp);
			int port;
			address += readInt(address, &port);
			setMqttPort(port);
			address += readString(address, &temp);
			setMqttClientId(temp);
			address += readString(address, &temp);
			setMqttPublishTopic(temp);
			address += readString(address, &temp);
			setMqttSubscribeTopic(temp);

			bool secure = false;
			address += readBool(address, &secure);
			if (secure)
			{
				address += readString(address, &temp);
				setMqttUser(temp);
				address += readString(address, &temp);
				setMqttPassword(temp);
			} else {
				setMqttUser("");
				setMqttPassword("");
			}
		} else {
			clearMqtt();
		}

		address += readByte(address, &authSecurity);
		if (authSecurity > 0) {
			address += readString(address, &temp);
			setAuthUser(temp);
			address += readString(address, &temp);
			setAuthPassword(temp);
		} else {
			clearAuth();
		}

		int i;
		address += readInt(address, &i);
		setUnitBaud(i);
		address += readInt(address, &i);
		setUnitId(i);

		ackWifiChange();

		success = true;
    }
	return success;
}

bool configuration::save() {
	int address = EEPROM_CONFIG_ADDRESS;

	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(address, EEPROM_CHECK_SUM);
	address++;

	address += saveString(address, wifiSsid.c_str());
	address += saveString(address, wifiPassword.c_str());
	address += saveString(address, wifiIp.c_str());
	address += saveString(address, wifiGw.c_str());
	address += saveString(address, wifiSubnet.c_str());
	if(!mqttHost.isEmpty()) {
		address += saveBool(address, true);
		address += saveString(address, mqttHost.c_str());
		address += saveInt(address, mqttPort);
		address += saveString(address, mqttClientId.c_str());
		address += saveString(address, mqttPublishTopic.c_str());
		address += saveString(address, mqttSubscribeTopic.c_str());
		if (!mqttUser.isEmpty()) {
			address += saveBool(address, true);
			address += saveString(address, mqttUser.c_str());
			address += saveString(address, mqttPassword.c_str());
		} else {
			address += saveBool(address, false);
		}
	} else {
		address += saveBool(address, false);
	}

	address += saveByte(address, authSecurity);
	if (authSecurity > 0) {
		address += saveString(address, authUser.c_str());
		address += saveString(address, authPassword.c_str());
	}

	address += saveInt(address, unitBaud);
	address += saveInt(address, unitId);

	bool success = EEPROM.commit();
	EEPROM.end();

	return success;
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

int configuration::saveString(int pAddress, const char* pString) {
	int address = 0;
	int length = pString ? strlen(pString) + 1 : 0;
	EEPROM.put(pAddress + address, length);
	address++;

	for (int i = 0; i < length; i++)
	{
		EEPROM.put(pAddress + address, pString[i]);
		address++;
	}

	return address;
}

int configuration::readInt(int address, int *value) {
	int lower = EEPROM.read(address);
	int higher = EEPROM.read(address + 1);
	*value = lower + (higher << 8);
	return 2;
}

int configuration::saveInt(int address, int value) {
	byte lowByte = value & 0xFF;
	byte highByte = ((value >> 8) & 0xFF);

	EEPROM.write(address, lowByte);
	EEPROM.write(address + 1, highByte);

	return 2;
}

int configuration::readBool(int address, bool *value) {
	byte y = EEPROM.read(address);
	*value = (bool)y;
	return 1;
}

int configuration::saveBool(int address, bool value) {
	byte y = (byte)value;
	EEPROM.write(address, y);
	return 1;
}

int configuration::readByte(int address, byte *value) {
	*value = EEPROM.read(address);
	return 1;
}

int configuration::saveByte(int address, byte value) {
	EEPROM.write(address, value);
	return 1;
}

template <class T> int configuration::writeAnything(int ee, const T& value) {
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.write(ee++, *p++);
	return i;
}

template <class T> int configuration::readAnything(int ee, T& value) {
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}
