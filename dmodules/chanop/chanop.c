#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#include "acidblood.h"
#include "list.h"
#include "events.h"
#include "config.h"
#include "acidfuncs.h"
#include "internal.h"
#include "tracking.h"
#include "extern.h"

static void user_join(int UNUSED(event), struct socket_info *sinfo, char *nick, char *dest, char *UNUSED(info))
{
	int uaccess;
	struct server_list *server ;
	struct userlist *user ;
	char *channel = dest ;
	
	if (botinfo->autoop != 1) {
		return;
	}

	server = GetServerBySocket(sinfo) ;

	user = GetFromChannel(nick, channel, server);
	uaccess = GetChannelAccess(user, channel);

	if (uaccess <= botinfo->oplevel) {
		info_log("Auto op %s\n", nick);
		irc_sprintf(sinfo, "MODE %s +o %s\n", channel, nick);
		return;
	}

	if (uaccess <= botinfo->hoplevel) {
		info_log("Auto hop %s\n", nick);
		irc_sprintf(sinfo, "MODE %s +h %s\n", channel, nick);
		return;
	}

	if (uaccess <= botinfo->voicelevel) {
		info_log("Auto voice %s\n", nick);
		irc_sprintf(sinfo, "MODE %s +v %s\n", channel, nick);
		return;
	}
}

static int msg_op(const char *replyto, struct userlist *user, char *data)
{
	const char *channel, *password, *nick;
	struct socket_info *sinfo ;
	int user_access;

	channel = NULL;
	password = NULL;

	if (data != NULL) {

		/* syntax: op <channel> <password> */

		channel = strtok(data, " ");
		password = strtok(NULL, "");
	}

	if (channel == NULL) {
		if (replyto[0] == '#') {
			channel = replyto;
		} else {
			msgreply(user, replyto, "Syntax error: op <channel> <password>");
			return (0);
		}
	}

	if (password != NULL)
		UpdateAccess(user, password);

	user_access = GetChannelAccess(user, channel);

	if (user_access < 0) {
		msgreply(user, replyto, "Authorization failed!");
		return (0);
	}

	sinfo = usinfo(user) ;
	nick = unick(user) ;
	
	if (user_access <= botinfo->oplevel) {
		info_log("Op %s on %s\n", nick, channel);
		msgreply(user, replyto, "Opping %s on %s", nick, channel);
		irc_sprintf(sinfo, "MODE %s +o %s\n", channel, nick);

		return (0);
	}

	if (user_access <= botinfo->hoplevel) {
		info_log("HOp %s on %s\n", nick, channel);
		msgreply(user, replyto, "Half opping %s on %s", nick, channel);
		irc_sprintf(sinfo, "MODE %s +h %s\n", channel, nick);

		return (0);
	}

	if (user_access <= botinfo->voicelevel) {
		info_log("Voice %s on %s\n", nick, channel);
		msgreply(user, replyto, "Voiceing %s on %s", nick, channel);
		irc_sprintf(sinfo, "MODE %s +v %s\n", channel, nick);

		return (0);
	}

	msgreply(user, replyto, "Authorization failed!");
	return (0);
}

static int msg_opuser(const char *replyto, struct userlist *user, char *data)
{
	char *channel, *usertoop ;

	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: opuser <channel> <user>");
		return (0);
	}

	channel = strtok(data, " ");
	usertoop = strtok(NULL, "");

	if (user == NULL) {
		msgreply(user, replyto, "Syntax error: opuser <channel> <user>");
		return (0);
	}

	if (GetChannelAccess(user, channel) >= 0) {
		info_log("Opuser %s on %s\n", usertoop, channel);
		msgreply(user, replyto, "Opping %s on %s", usertoop, channel);
		irc_sprintf(usinfo(user), "MODE %s +o %s\n", channel, usertoop);
	} else {
		msgreply(user, replyto, "Authorization failed!");
	}

	return (0);
}


static int msg_deop(const char *replyto, struct userlist *user, char *data)
{

	const char *channel, *nick, *snick, *tnick;
	struct socket_info *sinfo ;
	struct userlist *target;
	int user_access;

	channel = NULL;
	nick = NULL;

	if (data != NULL) {

		/* syntax: deop <channel> <nick> */

		channel = strtok(data, " ");
		nick = strtok(NULL, "");
	}

	if (channel == NULL) {
		if (replyto[0] == '#') {
			channel = replyto;
		} else {
			msgreply(user, replyto, "Syntax error: deop <channel> <nick>");
			return (0);
		}
	}

	user_access = GetChannelAccess(user, channel);

	if (user_access < 0) {
		return (access_too_low(user, replyto));
	}

	sinfo = usinfo(user) ;
	
	if (nick == NULL) {
		irc_sprintf(sinfo, "MODE %s -o %s\n", channel, unick(user));
		return(0) ;
	}

	target = GetFromChannel(nick, channel, user->userinfo->server);
	tnick = unick(target) ;

	snick = unick(user) ;

	if (target == NULL) {
		msgreply(user, replyto, "I don't see that user in the channel");
		return (0);
	}
		
	if ((target->access >= 0) && (user_access > target->access)) {
		privmsg(sinfo, tnick, "%s tried to deop you!", snick);
		msgreply(user, replyto, "attempt to deop user with higher access denied");
		return (0);
	}

	info_log("Deop %s on %s\n", snick, channel);
	msgreply(user, replyto, "Deopping %s on %s", tnick, channel);
	irc_sprintf(sinfo, "MODE %s -o %s\n", channel, tnick);
	return (0);
}

static int msg_topic(const char *replyto, struct userlist *user, char *data)
{
	char *channel, *topic ;

	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: topic <channel> <text>");
		return (0);
	}

	channel = strtok(data, " ");
	topic = strtok(NULL, "");

	if (channel == NULL || topic == NULL) {
		msgreply(user, replyto, "Syntax error: topic <channel> <text>");
		return (0);
	}

	if (GetChannelAccess(user, channel) < 0) {
		return (access_too_low(user, replyto));
	}
	info_log("Topic on %s set to %s by %s\n", channel, topic, unick(user));

	msgreply(user, replyto, "Changing topic on %s", channel);

	irc_sprintf(usinfo(user), "TOPIC %s :%s\n", channel, topic);

	return (0);

}

static int msg_mode(const char *replyto, struct userlist *user, char *data)
{
	char *channel, *modes ;

	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: mode <channel> <flags>");
		return (0);
	}

	channel = strtok(data, " ");
	modes = strtok(NULL, "");

	if (channel == NULL || modes == NULL) {
		msgreply(user, replyto, "Syntax error: mode <channel> <flags>");
		return (0);
	}

	if (GetChannelAccess(user, channel) < 0) {
		return (access_too_low(user, replyto));
	}
	info_log("Mode change %s on %s by %s\n", modes, channel, unick(user));
	msgreply(user, replyto, "Changing mode on %s to %s", channel, modes);
	irc_sprintf(usinfo(user), "MODE %s %s\n", channel, modes);

	return (0);
}

static int msg_join(const char *replyto, struct userlist *user, char *data)
{
	char *channel, *key ;

	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: join <channel> <key>");
		return (0);
	}

	channel = strtok(data, " ");
	key = strtok(NULL, "");

	if (channel == NULL) {
		msgreply(user, replyto, "Syntax error: join <channel> <key>");
		return (0);
	}

	if (EnterChannel(data, key, user->userinfo->server) == -1) {
		msgreply(user, replyto, "I'm in that channel already");
	} else {
		msgreply(user, replyto, "Joining %s", channel);
		info_log("Join to %s requested by %s\n", channel, unick(user)) ;
	}

	return (0);
}

static int msg_part(const char *replyto, struct userlist *user, const char *data)
{

	if (data == NULL) {
		msgreply(user, replyto, "Syntax error: part <channel>");
		return (0);
	}

	if (ExitChannel(data, user->userinfo->server) == -1) {
		msgreply(user, replyto, "I'm not in channel %s", data);
	} else {
		msgreply(user, replyto, "Parting %s", data);
		info_log("Parted %s by %s\n", data, unick(user)) ;
	}

	return (0);

}

static int msg_autoop(const char *replyto, struct userlist *user, char *data)
{

	if (data == NULL) {
		if (botinfo->autoop == 1) {
			msgreply(user, replyto, "Autoop ON");
		} else {
			msgreply(user, replyto, "Autoop OFF");
		}

		return (0);
	}

	if (strcaseeq("off", data)) {
		botinfo->autoop = 0;
		msgreply(user, replyto, "Autoop OFF");
		return (0);
	}
	if (strcaseeq("on", data)) {
		botinfo->autoop = 1;
		msgreply(user, replyto, "Autoop ON");
		return (0);
	}

	msgreply(user, replyto, "Syntax error: Autoop <on/off>");
	return (0);
}

static int msg_kick(const char *replyto, struct userlist *user, char *data)
{

	char *channel, *nick, *message ;
	struct userlist *target;
	int user_access;

	/* syntax: kick <channel> <nick> */
	channel = strtok(data, " ");
	nick = strtok(NULL, " ");
	message = strtok(NULL, "");

	if (channel == NULL || nick == NULL) {
		msgreply(user, replyto, "Syntax error: kick <channel> <nickname>");
		return (0);
	}

	if (strcaseeq(nick, user->userinfo->server->nick)) {
		info_log("%s tried to make me kick myself\n", unick(user));
		irc_sprintf(usinfo(user), "KICK %s %s :Try to make me kick myself eh?\n", channel, unick(user));
		return (0);
	}

	user_access = GetChannelAccess(user, channel);

	if (user_access < 0)
		return (access_too_low(user, replyto));

	target = GetFromChannel(nick, channel, user->userinfo->server);

	if (target == NULL)
		msgreply(user, replyto, "I don't see that user in the channel");

	if ((target->access >= 0) && (user_access > target->access)) {
		privmsg(usinfo(user), unick(target), "%s tried to kick you!", unick(user) );
		privmsg(usinfo(user), replyto, "attempt to kick user with higher access denied");
		return (0);
	}

	info_log("KICK %s %s\n", channel, unick(target));

	msgreply(user, replyto, "Kicking %s on %s", nick, channel);

	if (message == NULL) {
		irc_sprintf(usinfo(user), "KICK %s %s\n", channel, nick);
		return (0);
	} else {
		irc_sprintf(usinfo(user), "KICK %s %s :%s\n", channel, nick, message);
		return (0);
	}
}

int module_init(void *module)
{
	addrwusrcommand(module, "DEOP", msg_deop, 99);
	addrwusrcommand(module, "OP", msg_op, 99);
	addrwusrcommand(module, "TOPIC", msg_topic, 99);
	addrwusrcommand(module, "MODE", msg_mode, 99);
	addrwusrcommand(module, "JOIN", msg_join, 0);
	addusrcommand(module, "PART", msg_part, 0);
	addrwusrcommand(module, "AUTOOP", msg_autoop, 0);
	addrwusrcommand(module, "KICK", msg_kick, 99);
	addrwusrcommand(module, "OPUSER", msg_opuser, 0);
	addtrigger(module, EV_JOIN, user_join);
	return (0);
}

int module_shutdown(void *module)
{
	delusrcommand(module, "DEOP");
	delusrcommand(module, "OP");
	delusrcommand(module, "TOPIC");
	delusrcommand(module, "MODE");
	delusrcommand(module, "JOIN");
	delusrcommand(module, "PART");
	delusrcommand(module, "AUTOOP");
	delusrcommand(module, "KICK");
	delusrcommand(module, "OPUSER");
	deltrigger(module, EV_JOIN);
	return (0);
}
