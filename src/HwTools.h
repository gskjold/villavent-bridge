#ifndef _HWTOOLS_H
#define _HWTOOLS_H

#include "Arduino.h"

#include <WiFi.h>

#include "configuration.h"

#define LED_INTERNAL 0
#define LED_RED 1
#define LED_GREEN 2
#define LED_BLUE 3
#define LED_YELLOW 4

class HwTools {
public:
    void setup(GpioConfig*);
    double getVcc();
    int getWifiRssi();
    bool ledOn(uint8_t color);
    bool ledOff(uint8_t color);
    bool ledBlink(uint8_t color, uint8_t blink);

    HwTools() {};
private:
    GpioConfig* config;

    bool writeLedPin(uint8_t color, uint8_t state);
};

#endif
