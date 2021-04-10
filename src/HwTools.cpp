#include "HwTools.h"

void HwTools::setup(GpioConfig* config) {
    this->config = config;

    if(config->vccPin > 0 && config->vccPin < 40) {
        pinMode(config->vccPin, INPUT);
    } else {
        config->vccPin = 0xFF;
    }

    if(config->ledPin > 0 && config->ledPin < 40) {
        pinMode(config->ledPin, OUTPUT);
        ledOff(LED_INTERNAL);
    } else {
        config->ledPin = 0xFF;
    }

    if(config->ledPinRed > 0 && config->ledPinRed < 40) {
        pinMode(config->ledPinRed, OUTPUT);
        ledOff(LED_RED);
    } else {
        config->ledPinRed = 0xFF;
    }

    if(config->ledPinGreen > 0 && config->ledPinGreen < 40) {
        pinMode(config->ledPinGreen, OUTPUT);
        ledOff(LED_GREEN);
    } else {
        config->ledPinGreen = 0xFF;
    }

    if(config->ledPinBlue > 0 && config->ledPinBlue < 40) {
        pinMode(config->ledPinBlue, OUTPUT);
        ledOff(LED_BLUE);
    } else {
        config->ledPinBlue = 0xFF;
    }
}

double HwTools::getVcc() {
    double volts = 0.0;
    if(config->vccPin != 0xFF) {
        #if defined(ESP8266)
            volts = (analogRead(config->vccPin) / 1024.0) * 3.3;
        #elif defined(ESP32)
            volts = (analogRead(config->vccPin) / 4095.0) * 3.3;
        #endif
    } else {
        #if defined(ESP8266)
            volts = ((double) ESP.getVcc()) / 1024.0;
        #endif
    }

    float vccOffset = config->vccOffset / 100.0;
    float vccMultiplier = config->vccMultiplier / 1000.0;
    return vccOffset + (volts > 0.0 ? volts * vccMultiplier : 0.0);
}

int HwTools::getWifiRssi() {
    int rssi = WiFi.RSSI();
    return isnan(rssi) ? -100.0 : rssi;
}

bool HwTools::ledOn(uint8_t color) {
    if(color == LED_INTERNAL) {
        return writeLedPin(color, config->ledInverted ? LOW : HIGH);
    } else {
        return writeLedPin(color, config->ledRgbInverted ? LOW : HIGH);
    }
}

bool HwTools::ledOff(uint8_t color) {
    if(color == LED_INTERNAL) {
        return writeLedPin(color, config->ledInverted ? HIGH : LOW);
    } else {
        return writeLedPin(color, config->ledRgbInverted ? HIGH : LOW);
    }
}

bool HwTools::ledBlink(uint8_t color, uint8_t blink) {
    for(int i = 0; i < blink; i++) {
        if(!ledOn(color)) return false;
        delay(50);
        ledOff(color);
        if(i != blink)
            delay(50);
    }
}

bool HwTools::writeLedPin(uint8_t color, uint8_t state) {
    switch(color) {
        case LED_INTERNAL: {
            if(config->ledPin != 0xFF) {
                digitalWrite(config->ledPin, state);
                return true;
            } else {
                return false;
            }
            break;
        }
        case LED_RED: {
            if(config->ledPinRed != 0xFF) {
                digitalWrite(config->ledPinRed, state);
                return true;
            } else {
                return false;
            }
            break;
        }
        case LED_GREEN: {
            if(config->ledPinGreen != 0xFF) {
                digitalWrite(config->ledPinGreen, state);
                return true;
            } else {
                return false;
            }
            break;
        }
        case LED_BLUE: {
            if(config->ledPinBlue != 0xFF) {
                digitalWrite(config->ledPinBlue, state);
                return true;
            } else {
                return false;
            }
            break;
        }
        case LED_YELLOW: {
            if(config->ledPinRed != 0xFF && config->ledPinGreen != 0xFF) {
                digitalWrite(config->ledPinRed, state);
                digitalWrite(config->ledPinGreen, state);
                return true;
            } else {
                return false;
            }
            break;
        }
    }
    return false;
}
