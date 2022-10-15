#ifndef BATD_DEBUG_H
#define BATD_DEBUG_H
#include <syslog.h>

void debug_print(int level, const char *format, ...);

#endif
