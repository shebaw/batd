#include <syslog.h>
#include <stdarg.h>

static void debug_print(int level, const char *format, ...)
{
	va_list va;
	va_start(va, format);
	vsyslog(LOG_DAEMON | level, format, va);
	va_end(va);
}

