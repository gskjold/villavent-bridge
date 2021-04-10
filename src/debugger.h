#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#include <stdarg.h>
#include <stdio.h>

size_t debugD(const char*, ...);
size_t debugI(const char*, ...);
size_t debugW(const char*, ...);
size_t debugE(const char*, ...);

#endif
