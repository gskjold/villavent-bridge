#include "debugger.h"
#include "Arduino.h"

size_t debugD(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256];text[sizeof(text)-1]=0;
	    vsnprintf(text, sizeof(text), format, arg);
        Serial.printf("[DEBUG] %s\r\n", text);
    #endif
    return 0;
}

size_t debugI(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256];text[sizeof(text)-1]=0;
	    vsnprintf(text, sizeof(text), format, arg);
        Serial.printf("[INFO] %s\r\n", text);
    #endif
    return 0;
}

size_t debugW(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256];text[sizeof(text)-1]=0;
	    vsnprintf(text, sizeof(text), format, arg);
        Serial.printf("[WARNING] %s\r\n", text);
    #endif
    return 0;
}

size_t debugE(const char* format, ...) {
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256];text[sizeof(text)-1]=0;
	    vsnprintf(text, sizeof(text), format, arg);
        Serial.printf("[ERROR] %s\r\n", text);
    #endif
    return 0;
}
