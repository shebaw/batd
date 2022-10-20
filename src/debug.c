#include <stdio.h>
#include <stdarg.h>

void debug_printf(int level, const char *format, ...)
{
	va_list va;
	va_start(va, format);
	vfprintf(stderr, format, va);
	va_end(va);
}
