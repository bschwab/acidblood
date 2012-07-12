#include "acidblood.h"
#include "acidfuncs.h"
#include "tracking.h"
#include "internal.h"
#include "extern.h"
#include "startup.h"

#include <stdarg.h>
#include <string.h>

#define CTCP_DELIM_CHAR '\001'

void addctcp(void *module, const char *command, int (*function) (struct userlist *, const char *))
{
	ctcplist = addcommand(module,ctcplist, command, function, 99, 0);
}

void delctcp(void *module, const char *command)
{
	ctcplist = delcommand(module,ctcplist, command);
}

void delctcpmodule(void *module)
{
	ctcplist = delcommandmodule(module,ctcplist);
}

void send_ctcp(struct socket_info *sinfo, const char *sendto, const char *format, ...)
{
	static char output[1000];
	va_list args;

	va_start(args, format);
	vsnprintf(output,1000, format, args);
	va_end(args);
	send_notice(sinfo, sendto,"%c%s%c",CTCP_DELIM_CHAR, output, CTCP_DELIM_CHAR);
}

void send_action(struct socket_info *sinfo, const char *sendto, const char *format, ...)
{
	static char output[1000];
	va_list args;

	va_start(args, format);
	vsnprintf(output, 1000, format, args);
	va_end(args);
	privmsg(sinfo, sendto,"%cACTION %s%c",CTCP_DELIM_CHAR, output, CTCP_DELIM_CHAR);
}

void ctcp_reply(struct userlist *user, const char *format, ...)
{
	static char output[1000];
	va_list args;

	va_start(args, format);
	vsnprintf(output,1000, format, args);
	va_end(args);
	send_notice(user->userinfo->server->sinfo, user->userinfo->nick,"%c%s%c",CTCP_DELIM_CHAR, output, CTCP_DELIM_CHAR);
}

static int ctcp_ping(struct userlist *user, const char *data)
{
	time_t ping_t;

	if (data == NULL) {
		/* if no params send the time in seconds */
		time(&ping_t);
		ctcp_reply(user, "PING %li", (long) ping_t);
		return(0) ;
	}

	/* if an argument exists, send the same one back */
	if (strlen(data) > 100) {
		/* someone is trying to overflow us */
		return (0);
	}

	ctcp_reply(user, "PING %s", data);
	return(0) ;
}

static int ctcp_version(struct userlist *user, const char * UNUSED(data))
{
	ctcp_reply(user, "VERSION %s", botinfo->ver);
	return(0) ;
}

static int ctcp_time(struct userlist *user, const char * UNUSED(data))
{
	char timestr[50];

	time(&currtime);

	strftime(timestr, sizeof(timestr), "%a %b %d %H:%M:%Y",
		 localtime(&currtime));

	ctcp_reply(user, "TIME %s", timestr);
	return(0) ;
}

void init_ctcp()
{
	ctcplist = NULL;
	addctcp(NULL,"PING", ctcp_ping) ;
	addctcp(NULL,"VERSION", ctcp_version) ;
	addctcp(NULL,"TIME", ctcp_time) ;
}

int handle_ctcp(struct userlist *user, char *params)
{
	char *command;
	char *rest;
	int result ;
	struct commandlist *listptr;

	if(user->access < 0) {
		/* banned users get nothing */
		return(0) ;
	}
 
	if (botinfo->ctcp == 0) {
		return (0);
	}

	if(user->access > botinfo->floodexempt) {
		if(botinfo->ctcpdelay != 0 && botinfo->ctcpdelay+user->lastctcp >= currtime) {
			time(&user->lastctcp) ;
			return(0) ;
		}
	}
	
	time(&user->lastctcp) ;	
	strip_char_from_end(CTCP_DELIM_CHAR, params);
	command = strtok(params, " ");
	rest = strtok(NULL, "");
	
	listptr=ctcplist ;
	
	while(listptr!=NULL) {
		if (strcaseeq(command, listptr->cmd)) {
			result=listptr->function(user, rest);
			if(result<0) 
				return(result) ;
		}
		listptr = listptr->next ;
	}
	return (0);
}
