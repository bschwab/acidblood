/* user channel and server tracking info goes here */

#include <stdlib.h>
#include <string.h>
#include "acidblood.h"
#include "acidfuncs.h"
#include "tracking.h"
#include "internal.h"
#include "list.h"
#include "extern.h"

void DelChannel(struct channellist *, struct server_list *);

struct server_list *servers;

void InitTracking()
{
	servers = NULL;
}

static struct server_list *allocserverlist(void)
{
	struct server_list *slist;

	slist = malloc(sizeof(struct server_list) + 1);

	slist->next = NULL;
	slist->prev = NULL;

	return slist;
}

static struct channellist *allocchannellist(void)
{
	struct channellist *clist;

	clist = malloc(sizeof(struct channellist) + 1);

	clist->next = NULL;
	clist->prev = NULL;

	return clist;
}

static struct userlist *allocuserlist(void)
{
	struct userlist *ulist;

	ulist = malloc(sizeof(struct userlist) + 1);

	ulist->next = NULL;
	ulist->prev = NULL;

	return ulist;
}

static struct server_list *GetServer(const char *name)
{
	struct server_list *server_curr = NULL;

	if (servers == NULL) {
		return NULL;
	}

	server_curr = servers;

	while (server_curr != NULL) {
		if (strcaseeq(server_curr->name, name)) {
			return server_curr;
		}
		server_curr = server_curr->next;
	}

	return NULL;


}

struct server_list *GetServerBySocket(struct socket_info *sinfo)
{
	struct server_list *server_curr = NULL;

	if (servers == NULL) {
		return NULL;
	}

	server_curr = servers;

	while (server_curr != NULL) {
		if (server_curr->sinfo == sinfo) {
			return server_curr;
		}
		server_curr = server_curr->next;
	}

	return NULL;


}


void AddServer(struct socket_info *sinfo, char *name, char *nick)
{
	struct server_list *server_curr = NULL, *tmp_server;

	tmp_server = allocserverlist();

	LinkToList(tmp_server, server_curr, servers);

	server_curr->name = strdup(name);
	server_curr->nick = strdup(nick);
	server_curr->sinfo = sinfo;

	server_curr->clist = NULL;
	server_curr->ulist = NULL;
	server_curr->next = NULL;

	return;

}

const char *GetNick(struct socket_info *sinfo)
{

	struct server_list *server;

	server = GetServerBySocket(sinfo);

	if (servers == NULL) {
		return NULL;
	}
	
	return(server->nick) ;
}


void DelServer(struct server_list *server_curr)
{

	UnlinkListItem(server_curr, servers);
	/* we don't care about the channel and user lists anymore 
	   so we dump them as quickly as possible 
	 */

	if (server_curr->ulist != NULL) {
		while (server_curr->ulist->next != NULL) {
			server_curr->ulist = server_curr->ulist->next;
			FreeUser(server_curr->ulist->prev->userinfo);
			free(server_curr->ulist->prev);
		}
		FreeUser(server_curr->ulist->userinfo);
		free(server_curr->ulist);
	}
	if (server_curr->clist != NULL) {
		while (server_curr->clist->next != NULL) {
			server_curr->clist = server_curr->clist->next;
			FreeChannel(server_curr->clist->prev->channel);
			free(server_curr->clist->prev);
		}
		FreeChannel(server_curr->clist->channel);
		free(server_curr->clist);
	}
	FreeServer(server_curr);
}

void FreeServer(struct server_list *server_curr)
{
	free(server_curr->nick);
	free(server_curr->name);
	free(server_curr);
}

void DelServerByName(char *name)
{
	struct server_list *server_curr = NULL;

	server_curr = GetServer(name);

	if (server_curr == NULL) {
		debug_log("Attempted to remove non existant server: %s\n", name);
		return;
	}

	DelServer(server_curr);
}

void DelServerBySocket(struct socket_info *sinfo)
{
	struct server_list *server_curr = NULL;

	server_curr = GetServerBySocket(sinfo);

	if (server_curr == NULL)
		return;

	DelServer(server_curr);

}

static struct userlist *GetUser(struct clientinfo *client)
{
	struct server_list *server_curr = NULL;
	struct userlist *ulist = NULL;

	server_curr = client->server;

	if (server_curr == NULL) {
		return NULL;
	}
	if (server_curr->ulist == NULL) {
		return NULL;
	}

	ulist = server_curr->ulist;

	while (ulist != NULL) {
		if (strcaseeq(ulist->userinfo->nick, client->nick)) {
			return ulist;
		}
		ulist = ulist->next;
	}

	return NULL;
}


struct userlist *UpdateUser(struct clientinfo *client)
{
	struct userlist *ulist, *tmpulist;
	struct server_list *server_curr = NULL;

	time(&currtime);

	/* we don't want to track ourself */
	if (strcaseeq(client->nick, client->server->nick))
		return (NULL);

	ulist = GetUser(client);

	if (ulist == NULL) {
		server_curr = client->server;
		tmpulist = allocuserlist();

		LinkToList(tmpulist, ulist, server_curr->ulist);

		ulist->userinfo = malloc(sizeof(struct auser) + 1);
		ulist->userinfo->nick = strdup(client->nick);
		ulist->userinfo->ident = NULL;
		ulist->userinfo->address = NULL;
		ulist->userinfo->channels = NULL;
		ulist->userinfo->lastmsg = (int) currtime;
		ulist->userinfo->server = server_curr;
		ulist->access = -1;
		ulist->modes = 0;


	}
	/* not all sources of information come with 
	   ident and adress info.  This will let us pick 
	   up the info passively so as not to flood the 
	   server with requests for user info when say 
	   joining a large channel. 
	 */

	if (client->ident != NULL && ulist->userinfo->ident == NULL) {
		ulist->userinfo->ident = strdup(client->ident);
		ulist->userinfo->address = strdup(client->address);
		if (ulist->access < 0) {
			SetAccess(ulist, NULL);
		}

	}

	return (ulist);
}

struct userlist *AddUserToChannel(struct clientinfo *client, char *channel)
{
	struct userlist *ulist;

	time(&currtime);

	/* we don't want to track ourself */
	if (strcaseeq(client->nick, client->server->nick))
		return (NULL);

	ulist = UpdateUser(client);

	if (channel != NULL) {
		LinkUserToChannel(ulist->userinfo, channel);
	}

	return (ulist);
}

void RenameUser(struct clientinfo *client, char *newnick)
{
	struct userlist *ulist;

	/* we don't want to track ourself */
	if (strcaseeq(newnick, client->server->nick))
		return;

	ulist = GetUser(client);

	if (ulist == NULL) {
		debug_log("error: trying to rename unknown user: %s\n", client->nick);
		return;
	}

	free(ulist->userinfo->nick);
	ulist->userinfo->nick = strdup(newnick);
}

void LinkUserToChannel(struct auser *user, char *channel)
{
	struct channellist *clist, *uclist;
	struct userlist *ulist;
	clist = GetChannel(channel, user->server);

	if (clist == NULL) {
		clist = AddChannel(user, channel);
	}

	if (clist->channel->users == NULL) {
		clist->channel->users = allocuserlist();
		clist->channel->users->prev = NULL;
		clist->channel->users->next = NULL;
		clist->channel->users->userinfo = user;
		ulist = clist->channel->users;
	} else {
		ulist = clist->channel->users;
		FindEndOfList(ulist);
		ulist->next = allocuserlist();
		ulist->next->prev = ulist;
		ulist->next->next = NULL;
		ulist->next->userinfo = user;
		ulist = ulist->next;
	}

	SetChanAccess(ulist, channel, NULL);

	if (user->channels == NULL) {
		user->channels = allocchannellist();
		user->channels->prev = NULL;
		user->channels->next = NULL;
		user->channels->channel = clist->channel;
	} else {
		uclist = user->channels;
		FindEndOfList(uclist);
		uclist->next = allocchannellist();
		uclist->next->prev = uclist;
		uclist->next->next = NULL;
		uclist->next->channel = clist->channel;
	}

}

void DelUser(struct userlist *user)
{

	UnlinkListItem(user, user->userinfo->server->ulist);

	FreeUser(user->userinfo);
	free(user);
}

void DelUserFromChannel(struct clientinfo *client, char *channel)
{

	struct channellist *clist;
	struct userlist *ulist;

	/* we don't want to track ourself */
	if (strcaseeq(client->nick, client->server->nick))
		return;

	clist = GetChannel(channel, client->server);

	if (clist == NULL) {
		debug_log("Null clist pointer in DelUserFromChannel, nick: %s channel: %s\n", client->nick, channel);
		return;
	}

	ulist = GetFromChannel(client->nick, channel, client->server);

	if (ulist == NULL) {
		debug_log("Attempted to remove %s not in channel from %s\n", client->nick, channel);
		return;
	}

	UnlinkListItem(ulist, clist->channel->users);
	free(ulist);
	ulist = GetUser(client);

	clist = ulist->userinfo->channels;

	while (clist != NULL) {
		if (strcaseeq(clist->channel->name, channel)) {
			break;
		}

		clist = clist->next;
	}

	UnlinkListItem(clist, ulist->userinfo->channels);
	free(clist);

}

void QuitUser(struct clientinfo *client)
{
	struct userlist *ulist;
	ulist = GetUser(client);

	if (ulist == NULL) {
		debug_log("quituser on nonexistant user %s\n", client->nick);
		return;
	}

	while (ulist->userinfo->channels != NULL) {
		DelUserFromChannel(client, ulist->userinfo->channels->channel->name);
	}

	DelUser(ulist);
}

void FreeUser(struct auser *user)
{
	free(user->nick);
	free(user->ident);
	free(user->address);
	free(user);
}

struct channellist *GetChannel(const char *name, struct server_list *server_curr)
{
	struct channellist *clist = NULL;

	if (server_curr == NULL) {
		return NULL;
	}
	if (server_curr->clist == NULL) {
		return NULL;
	}

	clist = server_curr->clist;

	while (clist != NULL) {
		if (strcaseeq(clist->channel->name, name)) {
			return clist;
		}
		clist = clist->next;
	}
	return NULL;
}

int EnterChannel(const char *name, const char *key, struct server_list *server)
{
	struct channellist *clist;

	clist = GetChannel(name, server);
	if (clist != NULL)
		return (-1);

	insert_channels(name, key);

	if (key == NULL) {
		irc_sprintf(server->sinfo, "JOIN %s\n", name);
	} else {
		/* there is a key, send it also */
		irc_sprintf(server->sinfo, "JOIN %s %s\n", name, key);
	}

	return (0);
}

int ExitChannel(const char *name, struct server_list *server)
{
	struct channellist *clist;

	clist = GetChannel(name, server);

	if (clist == NULL)
		return (-1);

	irc_sprintf(server->sinfo, "PART %s\n", name);

	delete_channel(name);
	DelChannel(clist, server);
	return (0);
}

struct channellist *AddChannel(struct auser *user, const char *channel)
{
	struct channellist *clist;
	struct server_list *server;
	server = user->server;

	if (server->clist == NULL) {
		server->clist = allocchannellist();
		server->clist->prev = NULL;
		server->clist->next = NULL;
		clist = server->clist;
	} else {
		clist = user->server->clist;
		FindEndOfList(clist);
		clist->next = allocchannellist();
		clist->next->prev = clist;
		clist->next->next = NULL;
		clist = clist->next;
	}
	clist->channel = malloc(sizeof(struct achannel) + 1);
	clist->channel->name = strdup(channel);
	clist->channel->topic = NULL;
	clist->channel->users = NULL;

	return (clist);
}

void DelChannel(struct channellist *clist, struct server_list *server)
{

	struct clientinfo client;

	client.address = NULL;
	client.ident = NULL;
	client.server = server;

	if (clist == NULL)
		return;

	while (clist->channel->users != NULL) {
		client.nick = clist->channel->users->userinfo->nick;
		DelUserFromChannel(&client, clist->channel->name);
	}

	UnlinkListItem(clist, server->clist);
	FreeChannel(clist->channel);
	free(clist);
}

void FreeChannel(struct achannel *channel)
{
	free(channel->name);
	free(channel->topic);
	free(channel);
}

void ReapUserList()
{
	struct userlist *ulist, *tmp;
	struct server_list *slist;
	time_t ct;
	int rk, uk;
	int del;

	time(&ct);

	slist = servers;

	rk = ct - 60 * 5;
	uk = ct - 30;
	del = 0;

	while (slist != NULL) {
		ulist = slist->ulist;
		while (ulist != NULL) {
			if (ulist->userinfo->channels == NULL) {
				if (ulist->access >= 0 && ((int) ulist->userinfo->lastmsg < (int) rk)) {
					del = 1;
				} else if ((int) ulist->userinfo->lastmsg < (int) uk) {
					del = 1;
				}
			}
			tmp = ulist;
			ulist = ulist->next;

			if (del == 1) {
				DelUser(tmp);
				del = 0;
			}

		}
		slist = slist->next;
	}
}

void UpdateMsgTime(struct userlist *user)
{

	time(&user->userinfo->lastmsg);

}

int GetAccess(struct userlist *user)
{
	return (user->access);
}

int GetChannelAccess(struct userlist *user, const char *channel)
{
	struct channellist *clist;
	struct userlist *uclist;

	clist = GetChannel(channel, user->userinfo->server);

	if (clist == NULL) {
		debug_log("GetChannelAccess called on nonexistant channel %s\n", channel);
		return (-1);
	}
	uclist = clist->channel->users;

	while (uclist != NULL) {
		if (user->userinfo == uclist->userinfo)
			return (uclist->access);

		uclist = uclist->next;
	}

	return (-1);
}

struct userlist *GetFromChannel(const char *nick, const char *channel, struct server_list *server)
{
	struct channellist *clist;
	struct userlist *uclist;

	clist = GetChannel(channel, server);

	if (clist == NULL) {
		return (NULL);
	}

	uclist = clist->channel->users;
	while (uclist != NULL) {
		if (strcaseeq(uclist->userinfo->nick, nick))
			return (uclist);

		uclist = uclist->next;
	}

	return (NULL);
}

int IsInChannel(char *nick, char *channel, struct server_list *server)
{
	struct channellist *clist;
	struct userlist *uclist;

	clist = GetChannel(channel, server);

	if (clist == NULL) {
		return (FALSE);
	}

	uclist = clist->channel->users;
	while (uclist != NULL) {
		if (strcaseeq(uclist->userinfo->nick, nick))
			return (TRUE);

		uclist = uclist->next;
	}

	return (FALSE);
}
