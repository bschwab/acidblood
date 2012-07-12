#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "acidblood.h"
#include "acidfuncs.h"
#include "network.h"
#include "internal.h"
#include "tracking.h"
#include "events.h"
#include "extern.h"
#include "list.h"
#include "startup.h"

struct numerics {
	struct numerics *prev ;
	int numeric ;
	void *module ;
	void (*function)(socket_info *, char *) ;
	struct numerics *next ;
} ;

static struct numerics *numerics ;

void addnumeric(void *module, int numeric, void (*function)(socket_info *, char *))
{
	struct numerics *numeric_curr, *tmp_numeric ;
	
	tmp_numeric=malloc(sizeof(struct numerics)) ;
	
	LinkToList(tmp_numeric,numeric_curr,numerics) ;
	
	numeric_curr->module=module;
	numeric_curr->numeric=numeric ;
	numeric_curr->function=function ;

}

void delnumeric(void *module,int numeric) 
{
	struct numerics *numeric_curr ;
	
	numeric_curr=numerics ;
	
	while(numeric_curr!=NULL) {
		if(numeric_curr->module==module && 
		   numeric_curr->numeric==numeric) {
			UnlinkListItem(numeric_curr, numerics);
			free(numeric_curr) ;
			return ;
		}
		
		numeric_curr=numeric_curr->next ;
	}
}

void delnumericmodule(void *module) 
{
	struct numerics *numeric_curr ;
	numeric_curr=numerics ;
	
	while(numeric_curr!=NULL) {
		if(numeric_curr->module==module) {
			UnlinkListItem(numeric_curr, numerics);
			free(numeric_curr) ;
		}
		
		numeric_curr=numeric_curr->next ;
	}
}

static int isnickstartchar(char c)
{
	/* checks for characters that are permitted at the start of a nick */
	return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122)
		|| (c >= 91 && c <= 96) || (c >= 123 && c <= 125));
}

static void get_users_from_chan(struct socket_info *sinfo, char *params)
{
	char *channel;
	char cmode;
	struct clientinfo user;

	channel = strtok(params, " ");
	channel = strtok(NULL, " ");
	channel = strtok(NULL, " ");

	user.nick = strtok(NULL, " ");
	user.ident = NULL;
	user.address = NULL;
	user.server = GetServerBySocket(sinfo);

	if (user.nick[0] == ':')
		user.nick++;

	do {
		/* if it's not a char allowed at the beginning 
		   of the nick it's probably a mode char 
		 */

		if (!isnickstartchar(user.nick[0])) {
			cmode = user.nick[0];
			user.nick++;

		}

		AddUserToChannel(&user, channel);
		botevent(EV_HERE,sinfo,user.nick,channel,NULL) ;
		/* next User */
		user.nick = strtok(NULL, " ");

	} while (user.nick != NULL);
}

static void numeric_nickinuse(struct socket_info *sinfo, char * UNUSED(params))
{

	struct server_list *server ;
	
	server = GetServerBySocket(sinfo);
	
	/* Nick already in use */
	if (strcaseeq(server->nick, botinfo->altnick)) {
		info_log("Regular and alternate nick names are in use. Removing server.\n");
		NotConnectable(sinfo) ;
		disconnected(sinfo) ;
		return ;
	}
	
	info_log("Nickname is already in use, using alternate.\n");
	
	free(server->nick);
	server->nick = strdup(botinfo->altnick);

	irc_sprintf(sinfo, "NICK %s\n", server->nick);
	
}

static void numeric_userhost(struct socket_info *sinfo, char *data)
{
	struct userlist *user;
	clientinfo client;
	char *tmp;

	strip_char_from_end('\r', data);

	tmp = strtok(data, " ");

	client.nick = strtok(NULL, "=");
	strip_char_from_end(':', client.nick);
	strip_char_from_end('*', client.nick);

	client.ident = strtok(NULL, "@");
	strip_char_from_end('+', client.ident);

	client.address = client.address = strtok(NULL, " ");

	client.server = GetServerBySocket(sinfo);

	user = UpdateUser(&client);
	UpdateAccess(user, NULL);
}

static void numeric_ready(struct socket_info *sinfo, char *params)
{
	char *channel, *info;

	strtok(params, " "); /* strip of username */
	channel = strtok(NULL, " ");
	info    = strtok(NULL, "");

	strip_char_from_end(':', info);
	strip_char_from_end('\r', info);

	botevent(EV_ENTERED,sinfo,NULL,channel,info) ;

}

static void srvready(struct socket_info *sinfo, char * UNUSED(params))
{
	complete_connect(sinfo) ;	
}

static void ignore_numeric(struct socket_info * UNUSED(sinfo), char * UNUSED(params)) 
{

}

void init_numeric()
{
	addnumeric(NULL,001,srvready); 	/* connected */
	addnumeric(NULL,002,ignore_numeric) ;	/* server version and port */
	addnumeric(NULL,003,ignore_numeric) ;	/* server compile time */
	addnumeric(NULL,004,ignore_numeric) ;	/* supported modes */
	addnumeric(NULL,250,ignore_numeric) ;	/* max connection count */
	addnumeric(NULL,251,ignore_numeric) ;	/* current global users */
	addnumeric(NULL,252,ignore_numeric) ;	/* current global opers */
	addnumeric(NULL,254,ignore_numeric) ;	/* current chanels */
	addnumeric(NULL,255,ignore_numeric) ;	/* current local connections */
	addnumeric(NULL,302,numeric_userhost) ;	/* userhost reply */
	addnumeric(NULL,353,get_users_from_chan) ;	 
	addnumeric(NULL,366,numeric_ready) ;	/* end of channel names list */
	addnumeric(NULL,372,ignore_numeric) ;	/* Motd body */
	addnumeric(NULL,375,ignore_numeric) ;	/* Motd headder */
	addnumeric(NULL,376,ignore_numeric) ;	/* Motd end */
	addnumeric(NULL,422,ignore_numeric) ;	/* Motd missing */
	addnumeric(NULL,433,numeric_nickinuse);
	addnumeric(NULL,465,ignore_numeric) ; 	/* klined */

}

int handle_numeric(struct socket_info *sinfo, struct inputstruct *is)
{
	struct numerics *numeric_curr ;
	int numeric, found = 0 ;
	
	numeric=atoi(is->command) ;
	numeric_curr=numerics ;
	
	while(numeric_curr!=NULL) {
		if(numeric==numeric_curr->numeric) {
			found=1 ;
			numeric_curr->function(sinfo,is->params) ;
		}
		
		numeric_curr=numeric_curr->next ;
		
	}
	
	if(found) 
		return(0) ;
		
	debug_log("unknown numeric detected\n");
	debug_log("prefix=%s command=%s args=%s \n",
		   is->prefix, is->command, is->params);
		return (0);
}
