#include "acidblood.h"
#include "acidfuncs.h"
#include "internal.h"
#include "tracking.h"
#include "extern.h"
#include "startup.h"
#include "protocol.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CTCP_DELIM_CHAR '\001'

#define cmd_param_rw           0x0001

void addusrcommand(void *module, const char *command, int (*function)(const char *, struct userlist *, const char *), int user_access ) {
	usercommandlist=addcommand(module, usercommandlist, command, function, user_access, 0) ;

}

void addrwusrcommand(void *module, const char *command, int (*function)(const char *, struct userlist *, char *), int user_access ) {
	usercommandlist=addcommand(module, usercommandlist, command, function, user_access,cmd_param_rw) ;
}

void delusrcommand(void *module, const char *command) {
	usercommandlist=delcommand(module, usercommandlist, command) ;
}

void delusrcommandmodule(void *module) {
	usercommandlist=delcommandmodule(module, usercommandlist) ;
}

int access_too_low(struct userlist *user, const char *replyto)
{
	msgreply(user, replyto,
		 "Your access is too low to use this command.");
	return(0) ;

}

static int msg_shutdown(const char *replyto, struct userlist *user, const char * UNUSED(data))
{
	info_log("Shutdown by %s\n", user->userinfo->nick);

	msgreply(user, replyto, "Shutting down");
	
	irc_sprintf(user->userinfo->server->sinfo, "QUIT :Shutdown requested by %s\n",user->userinfo->nick);

	sleep(3) ;
	acid_shutdown(0);
	
	/* needed to shut the compilor up */
	return(0) ;
}

static int msg_version(const char *replyto, struct userlist *user, const char * UNUSED(data))
{
	msgreply(user, replyto, "Version: %s", botinfo->ver);
	return(0) ;
}

static int msg_time(const char *replyto, struct userlist *user, const char * UNUSED(data))
{
	char timestr[50];

	time(&currtime);

	strftime(timestr, sizeof(timestr), "%a %b %d %H:%M:%Y",
		 localtime(&currtime));

	msgreply(user, replyto, "The current time is: %s", timestr);
	return(0) ;
}

static int msg_reload(const char *replyto, struct userlist *user, const char * UNUSED(data))
{
	info_log("Reload by %s\n", user->userinfo->nick);
	acid_reload("reloading\n") ;
	msgreply(user, replyto, "Reload complete.");
	return(0) ;
}

#if 0 /* too ugly to live */
static int msg_help(const char *replyto, struct userlist *user, const char *data)
{
	char *line;
	FILE *fp_help;

	/* This is not ideal.. spending this much time inside a routine will
	   make it hard for the bot to handle outside events in a timely manor */

	if ((line = malloc(100)) == NULL) {
		display_error("execute: Malloc error!\n");
		return (-1);
	}
	if ((fp_help = fopen("../docs/help", "r")) == NULL) {
		info_log("Cant open help file! \n");
		free(line);
		msgreply(user, replyto, "Cant open help file! ");
		return(0) ;
	}
	while (fgets(line, 100, fp_help) != NULL) {
		if (msgreply(user, replyto, "%s", line) == -1) {
			free(line);
			return (-1);
		}
		/* give the server time to accept the input */
		sleep(1);
	}
	free(line);
	return (0);
}
#endif 

static int msg_identify(const char *replyto, struct userlist *user, const char *data)
{
	/* syntax: identify <password> */

	if(strcaseeq(data,"help")) {
		msgreply(user, replyto, "Useage: identify <password>");
		return(0) ;
	} 
	UpdateAccess(user,data) ;
			
        msguser (user, "Identification complete");
	
	return(0) ;
}

static int msg_access(const char *replyto, struct userlist *user, const char *UNUSED(data))
{
	/* syntax: identify <password> */

	UpdateAccess(user,NULL) ;
			
        msgreply(user, replyto, "Access level for %s is %i",user->userinfo->nick,user->access);
	
	return(0) ;
}

static int msg_say(const char *replyto, struct userlist *user, char *data)
{
	char *channel, *message ;

        if (data == NULL) {
                msgreply(user, replyto, "Syntax error: say <channel> <text>");
                return(0) ;
        }

	channel = strtok(data, " ");
	message = strtok(NULL, "");

	if (channel == NULL || message == NULL) {
		msgreply(user, replyto, "Syntax error: say <channel> <text>");
		return(0) ;
	}

	if (GetChannelAccess(user, channel) < 0) {
		return (access_too_low(user, replyto));
	}

	info_log("Say %s on %s by %s\n", message, channel, unick(user));

	msguser(user, "Saying message on %s", channel);
	privmsg(usinfo(user),channel, "%s", message);
	return(0) ;

}

static int msg_action(const char *replyto, struct userlist *user, char *data)
{
        char *channel, *message ;

        if (data == NULL) {
                msgreply(user, replyto, "Syntax error: action <channel> <text>");
                return(0) ;
        }

        channel = strtok(data, " ");
        message = strtok(NULL, "");

        if (channel == NULL || message == NULL) {
                msgreply(user, replyto,
                         "Syntax error: action <channel> <text>");
                return(0) ;
        }

        if (GetChannelAccess(user, channel) < 0) {
                return (access_too_low(user,replyto));
        }

        info_log("Action %s on %s by %s\n", message, channel, unick(user));

        msguser(user, "Performing action on %s", channel);
	irc_sprintf(usinfo(user), "PRIVMSG %s :%cACTION %s%c\n", channel, CTCP_DELIM_CHAR, message, CTCP_DELIM_CHAR);
        return(0) ;

}

static int msg_raw(const char *replyto, struct userlist *user, const char *data)
{

	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: raw <command>");
		return(0) ;
	}
	info_log("RAW: %s by %s\n", data, unick(user)) ;
	msguser(user, "Sending raw message> %s", data);
	irc_sprintf(usinfo(user),"%s\n", data);
	return(0) ;
}

static int msg_join(const char *replyto, struct userlist *user, char *data)
{
	char *channel, *key ;

        if (data == NULL) {
                msgreply(user, replyto, "Syntax error: join <channel> <key>");
        	return(0) ;
        }

	channel = strtok(data, " ");
	key = strtok(NULL, "");

	if (channel == NULL) {
		msgreply(user, replyto, "Syntax error: join <channel> <key>");
		return(0) ;
	}
	
	if(EnterChannel(data,key, user->userinfo->server)==-1) { 
		msgreply(user, replyto,"I'm in that channel already") ;
	} else {
		msgreply(user, replyto, "Joining %s", channel);
		info_log("Join to %s requested by %s\n", channel, unick(user));	
	}
		
	return(0) ;
}

static int msg_part(const char *replyto, struct userlist *user, const char *data)
{

	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: part <channel>");
		return(0) ;
	}

	if(ExitChannel(data, user->userinfo->server)==-1) {
		msgreply(user, replyto, "I can't part a channel that I'm not in") ;
	} else {
		msgreply(user, replyto, "Parting %s", data) ;
		info_log("Parted %s by %s\n", data, user->userinfo->nick) ;
	}

	return (0);

}

static int msg_ctcp(const char *replyto, struct userlist *user, const char *data)
{

	if (data == NULL) {
		if (botinfo->ctcp == 1) {
			msgreply(user, replyto, "CTCP ON");
		} else {
			msgreply(user, replyto, "CTCP OFF");
		}
		
		return(0) ;
	}
	
	if (strcaseeq("off", data)) {
		botinfo->ctcp = 0;
		msgreply(user, replyto, "CTCP OFF");
		return(0) ;
	}
	if (strcaseeq("on", data)) {
		botinfo->ctcp = 1;
		msgreply(user, replyto, "CTCP ON");
		return(0) ;
	}
	msgreply(user, replyto, "Syntax error: ctcp <on/off>");
	return(0) ;
}

static int msg_nick(const char *replyto, struct userlist *user, const char * data)
{
	struct server_list *server ;
	
	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: nick <nickname>");
		return(0) ;
	}
	
	server = user->userinfo->server ;
	
	info_log("NICK changed from %s to %s by %s\n", server->nick, data,
	    unick(user)) ;

	free(server->nick);
	server->nick = strdup(data);

	msgreply(user, replyto, "Changing nick to %s", server->nick);
	irc_sprintf(usinfo(user),"NICK %s\n", server->nick);
	return(0) ;
}

static int msg_stats(const char *replyto, struct userlist *user, const char * UNUSED(data))
{
	time_t now;
	int min;
	int hour;
	int day;
	time_t uptime;

	time(&now);

	uptime = now - botinfo->starttime;

	day = uptime / 86400;
	if (day > 0) {
		uptime = uptime - (day * 86400);
	}
	hour = uptime / 3600;
	if (hour > 0) {
		uptime = uptime - (hour * 3600);
	}
	min = uptime / 60;

	if(day==1) {
		msgreply(user, replyto, 
		        "up %i day, %i:%02i",day, hour, min);
		return(0) ;
	}
	
	msgreply(user, replyto, 
	        "up %i days %i:%02i",day, hour, min);
	
	return(0) ;

}

static int msg_loadmodule(const char *replyto, struct userlist *user, const char *data) 
{
#ifdef MODULES	
	if(loadmodule(data)) {
		msgreply(user, replyto, "Module loaded.") ;
		return(0) ;
	}

	msgreply(user, replyto, "Unable to load module. See acid.log for more info") ;
	return(0) ;
#else
	msgreply(user, replyto, "Modules are not supported in this install") ;
	return(0) ;
#endif
}

static int msg_listmodules(const char *replyto, struct userlist *user, const char * UNUSED(data)) 
{
#ifdef MODULES
	listmods(replyto, user) ;
	return(0) ;
#else
	msgreply(user, replyto, "Modules are not supported in this install") ;
	return(0) ;
#endif

}

static int msg_removemodule(const char *replyto, struct userlist *user, const char *data) 
{
#ifdef MODULES

	if(removemodule(data)) {
		msgreply(user, replyto, "Module removed.");
		return(0) ;
	}
	
	msgreply(user, replyto, "Module not found.");
	return(0) ;
#else
	msgreply(user, replyto, "Modules are not supported in this install") ;
	return(0) ;
#endif
}

static int msg_list(const char *replyto, struct userlist *user, const char * UNUSED(data)) 
{
	struct commandlist *listptr;
	char empty[]="" ;
	char *disp[4] ; 
	int i ;
	
	listptr = usercommandlist;
	
	for ( i = 0 ; i < 4 ; i++ ) {
		disp[i] = empty ;
	}
	
	i = 0 ;
	
	while (listptr != NULL) {
		if (user->access <= listptr->access) {
			disp[i] = listptr->cmd ;
			i++ ;
		}

		listptr = listptr->next ;

		if(i == 4) {
			msgreply(user, replyto, "%15s %15s %15s %15s", disp[0], disp[1], disp[2], disp[3]) ;

			for ( i = 0 ; i < 4 ; i++ ) {
				disp[i] = empty ;
			}

			i = 0 ;
		} 
	}
	msgreply(user, replyto, "%15s %15s %15s %15s", disp[0], disp[1], disp[2], disp[3])  ;
	return (0);
}

void init_usercommands()
{
	usercommandlist = NULL;
	addusrcommand(NULL,"SHUTDOWN", msg_shutdown, 0);
	addusrcommand(NULL,"IDENTIFY", msg_identify, 99) ;
	addusrcommand(NULL,"VERSION", msg_version, 99);
	addusrcommand(NULL,"TIME", msg_time, 99);
	addusrcommand(NULL,"RELOAD", msg_reload, 0);
	//addusrcommand(NULL,"HELP", msg_help, 99);
	addrwusrcommand(NULL,"SAY", msg_say, 20);
	addusrcommand(NULL,"RAW", msg_raw, 0);
	addrwusrcommand(NULL,"JOIN", msg_join, 0);
	addusrcommand(NULL,"PART", msg_part, 0);
	addusrcommand(NULL,"CTCP", msg_ctcp, 0);
	addusrcommand(NULL,"NICK", msg_nick, 0);
	addusrcommand(NULL,"STATS", msg_stats, 99);
	addusrcommand(NULL,"LOADMODULE", msg_loadmodule, 0);
	addusrcommand(NULL,"REMOVEMODULE",msg_removemodule,0);
	addusrcommand(NULL,"LISTMODULES",msg_listmodules,0) ; 
	addrwusrcommand(NULL,"ACTION", msg_action, 99);
	addusrcommand(NULL,"ACCESS", msg_access, 99);
	addusrcommand(NULL,"LIST", msg_list, 99) ;
}

int handle_command(struct userlist *user, const char *dest, char *message)
{
	const char *command;
	char *rest, *trest;
	const char *replyto;
	struct commandlist *listptr;
	int ret ;
	
	if(user->access < 0) {
		/* banned users get nothing */
		return(0) ;
	}
	if(user->access > botinfo->floodexempt) {
		if(botinfo->ctcpdelay != 0 && botinfo->cmddelay+user->lastcmd >= currtime) {
        		time(&user->lastcmd) ;
        		return(0) ;
		}
	}
	time(&user->lastcmd) ;
                        
	if (strcaseeq(dest, user->userinfo->server->nick))
		replyto = unick(user) ;
	else
		replyto = dest;
	
	command = strtok(message, " ");
	rest = strtok(NULL, "");

	listptr = usercommandlist;
	
	while (listptr != NULL) {
		if (strcaseeq(command, listptr->cmd)) {
			if (user->access <= listptr->access) {
				if ( rest != NULL && (listptr->flags & cmd_param_rw )) { 
					trest = strdup(rest) ;
					ret = listptr->function(replyto, user, trest) ;
					safefree(trest) ;
					return(ret) ;
				} else {
					ret = listptr->function(replyto, user, rest) ;
					return (ret) ;
				}
			} else {
				return (access_too_low(user, replyto));
			}
		}
		listptr = listptr->next ;

	}
	return (0);
}
