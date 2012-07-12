#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h>

#include "acidblood.h"
#include "acidfuncs.h"
#include "config.h"

extern void acid_shutdown(int);

FILE *fp_log = NULL;
FILE *fp_debug = NULL;

void init_log()
{
	if ((fp_log = fopen(LOG, "a")) == NULL) {
		fprintf(stderr, "Cannot open log file %s\n", LOG);
		acid_shutdown(-1);
	}
#ifdef DEBUG
	if ((fp_debug = fopen(DEBUGLOG, "a")) == NULL) {
		fprintf(stderr, "Cannot open debug file %s\n", DEBUGLOG);
		acid_shutdown(-1);
	}
#endif
}

void info_log(const char *format, ...)
{
	static char logbuf[1024];
	char timestr[50];
	va_list args;

	if(fp_log == NULL) {
		return ;
	}
	
	va_start(args, format);

	vsprintf(logbuf, format, args);
	va_end(args);

	time(&currtime);
	strftime(timestr, sizeof(timestr), "%b %d %H:%M:%Y", 
	                  localtime(&currtime));

	fprintf(fp_log, "%s: %s", timestr, logbuf);
#ifdef DEBUG
	fprintf(fp_debug, "%s: %s", timestr, logbuf);
	fflush(fp_debug) ;
#endif
	fflush(fp_log) ;
}

void debug_log(const char *format, ...)
{
#ifdef DEBUG
	static char debugbuf[1024];
	char timestr[50];
	va_list args;

	if(fp_debug == NULL) {
		return ;
	}

	va_start(args, format);
	vsprintf(debugbuf, format, args);
	va_end(args);

	time(&currtime);
	strftime(timestr, sizeof(timestr), "%b %d %H:%M:%Y", 
	                  localtime(&currtime));

	fprintf(fp_debug, "%s: %s", timestr, debugbuf);
	fflush(fp_debug) ;
#endif
}

void close_log()
{
	fflush(fp_log);
	fclose(fp_log);

#ifdef DEBUG
	fflush(fp_debug);
	fclose(fp_debug);
#endif

}

void display_error(const char *format, ...)
{
	static char logbuf[1024];
	char timestr[50];
	va_list args;

	if(fp_log == NULL) {
		return ;
	}

	va_start(args, format);

	vsprintf(logbuf, format, args);
	va_end(args);

	time(&currtime);

	strftime(timestr, sizeof(timestr), "%b %d %H:%M:%Y", 
	                  localtime(&currtime));

	fprintf(stderr, "%s:ERROR: %s", timestr, logbuf);
	fprintf(fp_log, "%s:ERROR: %s", timestr, logbuf);
	fflush(fp_log);
}
