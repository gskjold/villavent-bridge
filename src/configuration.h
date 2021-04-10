#ifndef _CONFIGURATION_h
#define _CONFIGURATION_h
#include <EEPROM.h>

#define EEPROM_SIZE 1024 * 3
#define EEPROM_CHECK_SUM 1 // Used to check if config is stored. Change if structure changes
#define EEPROM_CONFIG_ADDRESS 0

#define CONFIG_SYSTEM_START 8
#define CONFIG_WIFI_START 16
#define CONFIG_MQTT_START 224
#define CONFIG_WEB_START 648
#define CONFIG_NTP_START 784

struct SystemConfig {
	uint8_t boardType;
	uint16_t unitBaud;
	uint8_t unitId;
}; // 4

struct WiFiConfig {
	char ssid[32];
	char psk[64];
    char ip[15];
    char gateway[15];
    char subnet[15];
	char dns1[15];
	char dns2[15];
	char hostname[32];
	bool mdns;
}; // 204 

struct MqttConfig {
	char host[128];
	uint16_t port;
	char clientId[32];
	char publishTopic[64];
	char subscribeTopic[64];
	char username[64];
	char password[64];
	bool ssl;
}; // 419

struct WebConfig {
	uint8_t security;
	char username[64];
	char password[64];
}; // 129

struct NtpConfig {
	bool enable;
	bool dhcp;
	int16_t offset;
	int16_t summerOffset;
	char server[64];
}; // 70

struct GpioConfig {
	uint8_t apPin;
	uint8_t ledPin;
	bool ledInverted;
	uint8_t ledPinRed;
	uint8_t ledPinGreen;
	uint8_t ledPinBlue;
	bool ledRgbInverted;
	uint8_t vccPin;
	int16_t vccOffset;
	uint16_t vccMultiplier;
	uint8_t vccBootLimit;
};

class configuration {
public:
	bool hasConfig();
	int getConfigVersion();

	bool save();

	bool getSystemConfig(SystemConfig&);
	bool setSystemConfig(SystemConfig&);
	bool isSystemChanged();
	void ackSystemChange();

	bool getWiFiConfig(WiFiConfig&);
	bool setWiFiConfig(WiFiConfig&);
	void clearWifi(WiFiConfig&);
	void clearWifiIp(WiFiConfig&);
	bool isWifiChanged();
	void ackWifiChange();

	bool getMqttConfig(MqttConfig&);
	bool setMqttConfig(MqttConfig&);
	void clearMqtt(MqttConfig&);
	void setMqttChanged();
	bool isMqttChanged();
	void ackMqttChange();

	bool getWebConfig(WebConfig&);
	bool setWebConfig(WebConfig&);
	void clearAuth(WebConfig&);

	bool getNtpConfig(NtpConfig&);
	bool setNtpConfig(NtpConfig&);
	void clearNtp(NtpConfig&);
	bool isNtpChanged();
	void ackNtpChange();

	void clear();
	
protected:

private:
	uint8_t configVersion = 0;

	bool systemChanged = false, wifiChanged = false, mqttChanged = false, meterChanged = true, domoChanged, ntpChanged = true, entsoeChanged = false;

	bool loadConfig42(int address);

	int readString(int pAddress, char* pString[]);
	int readInt(int pAddress, int *pValue);
	int readBool(int pAddress, bool *pValue);
	int readByte(int pAddress, byte *pValue);
	template <class T> int readAnything(int ee, T& value);
};
#endif
