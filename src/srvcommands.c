#include "acidblood.h"
#include "acidfuncs.h"
#include "tracking.h"
#include "internal.h"
#include "extern.h"
#include "network.h"
#include "events.h"
#include "startup.h"

#include <string.h>
#include <stdlib.h>

#define CTCP_DELIM_CHAR '\001'

extern int handle_command(struct userlist *, char *, char *);

void addservercommand(void *module, const char *command, int (*function) (socket_info *, struct inputstruct *))
{
	servercommandlist = addcommand(module,servercommandlist, command, function, 99, 0);

}

void delservercommand(void *module, const char *command)
{
	servercommandlist = delcommand(module, servercommandlist, command);
}

void delservercommandmodule(void *module)
{
	servercommandlist = delcommandmodule(module,servercommandlist);
}

static int sm_topic(socket_info *sinfo, struct inputstruct *is)
{
	char *channel, *topic;
	clientinfo client;

	convert_clientinfo(sinfo,is->prefix, &client);

	channel = strtok(is->params, " ");	/* channel */
	topic = strtok(NULL, "");	/* topic */

	if (strlen(topic) <= 2)
		return(0) ; 

	strip_char_from_end(':', topic);
	strip_char_from_end('\r', topic);

	info_log("%s TOPIC %s by %s\n", channel, topic, client.nick);
	
	botevent(EV_TOPIC,sinfo,client.nick,channel,topic) ;	
	
	return (0);
}

static int sm_ping(socket_info *sinfo, struct inputstruct *is)
{
	irc_sprintf(sinfo,"PONG %s\n", is->params);
	return(0);
}

static int sm_pong(socket_info *sinfo, /*@unused@*/__attribute__((unused)) struct inputstruct *is)
{

	/* we got a pong from our ping in the timeout routine */
	/* reset last packet time */

	sinfo->pingtimeout=0 ;

	return 0;
}

static int sm_error(socket_info *sinfo, struct inputstruct *is)
{
	info_log("ERROR %s\n", is->params);
	disconnected(sinfo) ;
	return (0);
	
}

static int sm_kill(socket_info *sinfo, struct inputstruct *is) 
{
	info_log("Killed %s\n", is->params) ;
	disconnected(sinfo) ;
	return(0) ;
}

static int sm_join(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client;
	struct userlist *user;
	char *channel, *tmp;

	convert_clientinfo(sinfo,is->prefix, &client);

	channel = strtok(is->params, " ");
	tmp = strtok(NULL, "");

	strip_char_from_end(':', channel);
	strip_char_from_end('\r', channel);

	user = AddUserToChannel(&client, channel);

	if (user == NULL)
		return (0);

	info_log("%s JOIN %s\n", client.nick, channel);
	
	botevent(EV_JOIN, sinfo, client.nick, channel, NULL) ;
	
	return (0);
}

static int sm_part(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client;
	char *channel, *reason;

	convert_clientinfo(sinfo,is->prefix, &client);

	channel = strtok(is->params, " ");
	reason = strtok(NULL, "");

	strip_char_from_end(':', channel);
	strip_char_from_end('\r', channel);

	botevent(EV_PART,sinfo,client.nick,channel,reason) ;

	if (strcaseeq(client.nick, client.server->nick)) {
		/* we dont care if its our own nick */
		return(0) ;
	}
	
	DelUserFromChannel(&client, channel);
	
	if(reason==NULL) {
		info_log("%s PART %s\n", channel, client.nick);
	} else {
		info_log("%s PART %s (%s)\n", channel, client.nick,reason);
	}
	
	return (0);
}

static int sm_privmsg(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client;
	struct userlist *user;
	char *dest, *message;

	convert_clientinfo(sinfo,is->prefix, &client);

	dest = strtok(is->params, " ");
	message = strtok(NULL, "");

	strip_char_from_end(':', message);
	strip_char_from_end('\r', message);

	info_log("%s %s :%s\n", dest, client.nick, message);

	user = UpdateUser(&client);

	if (user == NULL)
		return (0);

	botevent(EV_MSG,sinfo,client.nick,dest,message) ;

	UpdateMsgTime(user);

	if(message[0]==botinfo->commandchar) {
		handle_command(user, dest, &message[1]);
		return(0) ;
	}
	                                                
	
	if (message[0]== CTCP_DELIM_CHAR) {
		handle_ctcp(user, message);
		return (0);
	} 
	
	if (strcaseeq(dest, client.server->nick)) {
		handle_command(user, dest, message);
	}
	
	return (0);	
}

static int sm_notice(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client;
	struct userlist *user;
	char *message, *dest, *p, *data, *final;

	if (is->prefix != NULL) {
		convert_clientinfo(sinfo,is->prefix, &client);
	}

   	dest = strtok(is->params, " "); /* message */
        message = strtok(NULL, "");     /* extra */

        strip_char_from_end(':', message);
        strip_char_from_end('\r', message);

	if (is->prefix == NULL) {
		/* ignore server messages */

		/* some irc servers will want a password if ident doesnt work */
		/* NOTICE AUTH :*** Ident broken or disabled, to continue to connect you must type /QUOTE PASS 37044 */
		/* this will parse out the password and send it to the server */
		/* TODO: move to its own section --subcube */

		debug_log("message> %s\n", message);

		p = strstr(message, "QUOTE PASS");

		if (p != NULL) {

			/* parse out the password */
			
			data = strtok(p, " ");
			final = NULL;
			while (data != NULL) {
				final = data;
				data = strtok(NULL, " ");
			}
			debug_log("sending: PASS %s\n", final);
			irc_sprintf(sinfo, "PASS %s\n", final);
		}

		return (0);
	}

	info_log("NOTICE from %s :%s\n", client.nick, message);

	user = UpdateUser(&client);

	if (user == NULL)
		return (0);

	UpdateMsgTime(user);

	botevent(EV_MSG,sinfo,client.nick,dest,message) ;

	if (message[0]== CTCP_DELIM_CHAR) {
		handle_ctcp(user, message);
		return (0);
	}
	

	// fixme: this should probably be in it's own module --InnerFIRE

	if (strcaseeq(client.nick, "NickServ")) {
		if (strstr(message,
			   "This nickname is registered and protected.") !=
		    NULL) {
			info_log("%s Sent password to %s\n", GetNick(sinfo), client.nick);
			privmsg(sinfo, client.nick, "IDENTIFY %s",
				botinfo->nspass);
		}
	}
	if (strstr(message,
		   "Password accepted - you are now recognized.") !=
	    NULL) {
		join_channels(sinfo);
	}

	return (0);
}

static int sm_quit(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client;

	convert_clientinfo(sinfo,is->prefix, &client);

	QuitUser(&client);
	
	botevent(EV_QUIT,sinfo,client.nick,NULL,is->params) ;
	
	if(is->params==NULL) { 
		info_log("QUIT by %s\n", client.nick);
	} else {
		info_log("QUIT by %s (%s)\n", client.nick,is->params);
	}
	
	return (0);
}

static int sm_mode(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client;
	char *channel, *modes;
	convert_clientinfo(sinfo,is->prefix, &client);

	channel = strtok(is->params, " ");
	modes = strtok(NULL, "");

	strip_char_from_end('\r', modes);
	
	botevent(EV_MODE,sinfo,client.nick,channel,modes) ;
	
	info_log("%s MODE %s by %s\n", channel, modes, client.nick);

	return (0);
}

static int sm_nick(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client;
	convert_clientinfo(sinfo,is->prefix, &client);
	strip_char_from_end(':', is->params);

	RenameUser(&client, is->params);
	
	botevent(EV_NICK,sinfo,client.nick,NULL,is->params) ;
	
	return (0);

}

static int sm_kick(socket_info *sinfo, struct inputstruct *is)
{
	clientinfo client, target ;
	char *reason, *nick, *channel, *tmp;

	convert_clientinfo(sinfo,is->prefix, &client);

	channel = strtok(is->params, " ");	/* channel */
	nick = strtok(NULL, ":");
	reason = strtok(NULL, "");

	strip_char_from_end(' ', nick);

	info_log("%s was kicked from %s by %s - %s\n", nick, client.nick, channel, reason);

	build_clientinfo(&target, client.server, nick);

	DelUserFromChannel(&target, channel);
	
	tmp=malloc(strlen(target.nick) + strlen(reason)+2) ;
	sprintf(tmp,"%s %s",target.nick,reason) ;

	botevent(EV_KICK,sinfo,client.nick,channel,reason) ;
	
	/* did they kick me? */
	if (strcaseeq(nick, client.server->nick)) {
		info_log("Kicked from %s\n", channel);
		irc_sprintf(sinfo,"JOIN %s\n", channel);
		return(0) ;
	}
	return (0);
}

void init_srvcommands()
{
	servercommandlist = NULL;
	
	addservercommand(NULL,"TOPIC", sm_topic);
	addservercommand(NULL,"PING", sm_ping);
	addservercommand(NULL,"PONG", sm_pong);
	addservercommand(NULL,"ERROR", sm_error);
	addservercommand(NULL,"JOIN", sm_join);
	addservercommand(NULL,"PART", sm_part);
	addservercommand(NULL,"PRIVMSG", sm_privmsg);
	addservercommand(NULL,"NOTICE", sm_notice);
	addservercommand(NULL,"QUIT", sm_quit);
	addservercommand(NULL,"MODE", sm_mode);
	addservercommand(NULL,"KICK", sm_kick);
	addservercommand(NULL,"NICK", sm_nick);
	addservercommand(NULL,"KILL", sm_kill);
}


int handle_server_command(socket_info *sinfo, struct inputstruct *is)
{
	struct commandlist *listptr;
	
	listptr=servercommandlist ;
	
	while (listptr!=NULL) {
		if (strcaseeq(is->command, listptr->cmd)) {
			return (listptr->function(sinfo, is));
		}
		listptr=listptr->next ;
	}

	debug_log("unknown command from server:\n");
	debug_log("prefix=%s command=%s arguement=%s \n",
		  is->prefix, is->command, is->params);

	return (0);
}
