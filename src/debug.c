#ifdef DEBUG
#include <stdio.h>
#else
#include <syslog.h>
#endif
#include <stdarg.h>

#ifdef DEBUG
void debug_print(int level, const char *format, ...)
{
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}
#else
void debug_print(int level, const char *format, ...)
{
	va_list va;
	va_start(va, format);
	vsyslog(LOG_DAEMON | level, format, va);
	va_end(va);
}
#endif
