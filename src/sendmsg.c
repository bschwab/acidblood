/* Acidblood network routines */
/*
Acidblood IRC Bot

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "acidblood.h"
#include "acidfuncs.h"
#include "internal.h"
#include "tracking.h"
#include "events.h"
#include "extern.h"
#include "protocol.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>


void irc_sprintf(struct socket_info *sinfo, const char *format, ...)
{
	static char output[256];
	static char input[1024];
	va_list args;

	va_start(args, format);
	vsprintf(input, format, args);
	va_end(args);

	snprintf(output, 255, "%s", input);

	send_packet(sinfo, output);
}

void send_notice(struct socket_info *sinfo, const char *target, const char *format,
		 ...)
{
	/* Use for short messages only.. the RFC limmits this
	 * to 256 chars TOTAL that means the total message must 
	 * be 256 MINUS the space needed for NOTICE and the nick.
	 */
	
	static char inbuf[1000];
	static char output[1024];
	va_list args;

	va_start(args, format);
	vsprintf(inbuf, format, args);
	va_end(args);

	snprintf(output, 255, "NOTICE %s :%s", target, inbuf);
	botevent(EV_MSG,sinfo,GetNick(sinfo),target,output) ;
	strcat(output,"\n") ;
	send_packet(sinfo, output);
}

void privmsg(struct socket_info *sinfo, const char *nick, const char *format, ...)
{
	/* We now break up long messages in order to be more 
	 * in line with the behavior most irc servers expect. 
	 */

	static char buff[4096], *buffout;
	static char output[512];
	static char prefix[100] ;
	int lenmod, msgsize, pos ;
	va_list args;
	
	snprintf(prefix, 100, "PRIVMSG %s :", nick) ;
	lenmod=253-strlen(prefix) ;
	
	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);
	
	pos=0 ;
	msgsize=strlen(buff) ;
	buffout=buff ;
	while(pos< msgsize) {
		snprintf(output, 254, "%s%s", prefix, buffout);
		botevent(EV_MSG,sinfo,GetNick(sinfo),nick,output) ;
		strcat(output,"\n") ;
		send_packet(sinfo, output);
		buffout+=lenmod ;
		pos+=lenmod ;
	} 
}

void msguser(struct userlist *user, const char *format, ...)
{
	static char buff[4096], *buffout;
	static char output[512];
	static char prefix[100] ;
	int lenmod, msgsize, pos ;
	va_list args;
	struct socket_info *sinfo ;
	
	sinfo = user->userinfo->server->sinfo ;
	snprintf(prefix, 100, "PRIVMSG %s :", user->userinfo->nick) ;
	lenmod=253-strlen(prefix) ;
	
	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);
	
	pos=0 ;
	msgsize=strlen(buff) ;
	buffout=buff ;
	while(pos< msgsize) {
		snprintf(output, 254, "%s%s", prefix, buffout);
		botevent(EV_MSG,sinfo,GetNick(sinfo), user->userinfo->nick,output) ;
		strcat(output,"\n") ;
		send_packet(sinfo, output);
		buffout+=lenmod ;
		pos+=lenmod ;
	}
}

void msgreply(struct userlist *user, const char *replyto, const char *format, ...)
{
	static char buff[4096], *buffout;
	static char output[512];
	static char prefix[100] ;
	int lenmod, msgsize, pos ;
	va_list args;
	struct socket_info *sinfo ;
	
	sinfo = user->userinfo->server->sinfo ;
	snprintf(prefix, 100, "PRIVMSG %s :", replyto) ;
	lenmod=253-strlen(prefix) ;
	
	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);
	
	pos=0 ;
	msgsize=strlen(buff) ;
	buffout=buff ;
	while(pos< msgsize) {
		snprintf(output, 254, "%s%s", prefix, buffout);
		botevent(EV_MSG,sinfo,GetNick(sinfo),replyto,output) ;
		strcat(output,"\n") ;
		send_packet(sinfo, output);
		buffout+=lenmod ;
		pos+=lenmod ;
	} 
}
