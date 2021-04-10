#include "debugger.h"
#include "Arduino.h"

size_t debugD(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[255];
	    snprintf_P(text, sizeof(text), format, arg);
        Serial.printf("[DEBUG] %s\n", text);
    #endif
}

size_t debugI(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[255];
	    snprintf_P(text, sizeof(text), format, arg);
        Serial.printf("[INFO] %s\n", text);
    #endif
}

size_t debugW(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[255];
	    snprintf_P(text, sizeof(text), format, arg);
        Serial.printf("[WARNING] %s\n", text);
    #endif
}

size_t debugE(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[255];
	    snprintf_P(text, sizeof(text), format, arg);
        Serial.printf("[ERROR] %s\n", text);
    #endif
}
