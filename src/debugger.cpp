#include "debugger.h"
#include "Arduino.h"

size_t debugD(const char* format, ...) {
    size_t cnt = 0;
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256]; text[sizeof(text)-1]=0;;
	    vsnprintf(text, sizeof(text)-1, format, arg);
        cnt = Serial.printf("[DEBUG] %s\r\n", text);
    #endif
    return cnt;
}

size_t debugI(const char* format, ...) {
    size_t cnt = 0;
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256]; text[sizeof(text)-1]=0;
	    vsnprintf(text, sizeof(text), format, arg);
        cnt =  Serial.printf("[INFO] %s\r\n", text);
    #endif
    return cnt;
}

size_t debugW(const char* format, ...) {
    size_t cnt = 0;
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256]; text[sizeof(text)-1]=0;
	    vsnprintf(text, sizeof(text), format, arg);
        cnt =  Serial.printf("[WARNING] %s\r\n", text);
    #endif
    return cnt;
}

size_t debugE(const char* format, ...) {
    size_t cnt = 0;
    #if DEBUG_MODE
        va_list arg;
        va_start(arg, format);
        char text[256]; text[sizeof(text)-1]=0;
	    vsnprintf(text, sizeof(text), format, arg);
        cnt =  Serial.printf("[ERROR] %s\r\n", text);
    #endif
    return cnt;
}
