#include <syslog.h>
#include <stdarg.h>

void __vsyslog(int, const char *, va_list);

void __syslog_chk(int priority, int flag, const char *message, ...)
{
	va_list ap;
	va_start(ap, message);
	__vsyslog(priority, message, ap);
	va_end(ap);
}

void __vsyslog_chk(int priority, int flag, const char *message, va_list ap)
{
	__vsyslog(priority, message, ap);
}
