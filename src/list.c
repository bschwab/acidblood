/* Acidblood link list routines */
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
#include "tracking.h"
#include "internal.h"
#include "extern.h"
#include "list.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void clear_channels()
{
	struct channels *channel_curr, *channel_tmp;

	if (conf_channels == NULL) {
		return ;
	}
	
	channel_curr=conf_channels ;
	
	while(channel_curr!=NULL) {
		channel_tmp=channel_curr ;
		channel_curr=channel_curr->next ;
		
		free(channel_tmp->name);
		if(channel_tmp->key!=NULL)
			free(channel_tmp->key) ;
		free(channel_tmp) ;
	}
	
	conf_channels = NULL ;
}

int insert_channels(const char *name, const char *key)
{
	struct channels *channel_curr;

	if (conf_channels == NULL) {
		conf_channels = malloc(sizeof(struct channels));
		conf_channels->prev = NULL;
		channel_curr = conf_channels;
	} else {
		channel_curr = conf_channels;
		FindEndOfList(channel_curr);
		channel_curr->next = malloc(sizeof(struct channels));
		channel_curr->next->prev = channel_curr;
		channel_curr = channel_curr->next;
	}
	channel_curr->next = NULL;

	channel_curr->name = strdup(name);

	channel_curr->key = NULL;

	if (key != NULL)
		channel_curr->key = strdup(key);

	return (0);
}


int delete_channel(const char *name)
{
	struct channels *channel_curr;

	channel_curr = conf_channels;

	while (channel_curr != NULL) {
		if (strcaseeq(channel_curr->name, name)) {
			break;
		}
		channel_curr = channel_curr->next;
	}

	if (channel_curr == NULL)
		return (-1);

	UnlinkListItem(channel_curr, conf_channels);
	free(channel_curr);

	return (0);
}

void join_channels(struct socket_info *sinfo)
{
	struct channels *channel_curr;
	channel_curr = conf_channels;

	while (channel_curr != NULL) {
		if (channel_curr->key != NULL) {
#ifdef DEBUG
			debug_log("join_channels: JOIN %s %s\n",
				  channel_curr->name, channel_curr->key);
#endif
			irc_sprintf(sinfo, "JOIN %s %s\n", channel_curr->name,
				    channel_curr->key);
		} else {
#ifdef DEBUG
			debug_log("join_channels: JOIN %s\n",
				  channel_curr->name);
#endif
			irc_sprintf(sinfo, "JOIN %s\n", channel_curr->name);
		}
		channel_curr = channel_curr->next;
	}

}

void SetAccess(struct userlist *user, const char *password)
{
	struct userdata *userdata_curr;
	userdata_curr = conf_userdata;
	
	user->access= 99 ;
	
	while (userdata_curr != NULL) {
		if (match(userdata_curr->usernick, user->userinfo->nick)) {
			if (user->userinfo->address == NULL) {
				irc_sprintf(user->userinfo->server->sinfo,"USERHOST %s\n",
					    user->userinfo->nick);
				return ;
			}

			if (match(userdata_curr->userip,
			     user->userinfo->address)) {
				

				if (!match("all", userdata_curr->userchan))
					return ;
				
				if (userdata_curr->userpass == NULL) {
					user->access=userdata_curr->userstatus ;
					return ;
				}
			
				if (botinfo->reqpass == 1) {
					if (password == NULL
				    	|| !streq(userdata_curr->userpass,
					      password))
						return ;
				}
				
				user->access= userdata_curr->userstatus ;
				return ;
			}
		}
		userdata_curr = userdata_curr->next;
	}
	return ;
}

void SetChanAccess(struct userlist *user, const char *channel, const char *password)
{
	struct userdata *userdata_curr;
	struct channellist *clist ;
	struct userlist *culist ;
	
	userdata_curr = conf_userdata;
	clist=GetChannel(channel,user->userinfo->server) ;

	if(clist==NULL) {
		debug_log("error: Attempted to connect user %s to non existing channel %s\n", user->userinfo->nick, channel) ;
		return ;
	}

	culist=GetFromChannel(user->userinfo->nick,channel,user->userinfo->server) ;
	culist->access= 99 ;

	while (userdata_curr != NULL) {
		if (match(userdata_curr->usernick, user->userinfo->nick)) {
			
			if (user->userinfo->address == NULL) {
				irc_sprintf(user->userinfo->server->sinfo,"USERHOST %s\n",
					    user->userinfo->nick);
				return ;
			}

			if (match(userdata_curr->userip, user->userinfo->address)) {

				if (!match("all", userdata_curr->userchan)) {
					if (!check_chan
					    (userdata_curr->userchan, channel))
						return ;
				}
			
				if (userdata_curr->userpass == NULL) {
					culist->access= userdata_curr->userstatus ;
					return ;
				}
				if (botinfo->reqpass == 1) {
					if (password == NULL
				    	|| !streq(userdata_curr->userpass,
						      password))
						return ;
				}
			
				culist->access = userdata_curr->userstatus ;
				return ;
			}
		}
		userdata_curr = userdata_curr->next;
	}
	return ;
}

void UpdateAccess(struct userlist *user, const char *password) {
	struct channellist *clist ;
	
	SetAccess(user,password) ;
       
        clist=user->userinfo->channels; 
        while(clist!=NULL) {
        	SetChanAccess(user,clist->channel->name,password) ;
		clist=clist->next ;
        }
}
        
int check_chan(const char *userchan, const char *serverchan)
{

	char temp[100];
	int x = 0;

	/* op this person on ALL channels */
	if (strcaseeq(userchan, "all")) {
		return (1);
	}

	/* channel matches, most likely only one channel next to their username */
	if (strcaseeq(userchan, serverchan)) {
		return (1);
	}

	while (*userchan != '\0') {

		/* found a comma, skip and compare */
		if (*userchan == ',') {
			temp[x] = '\0';
			if (strcaseeq(temp, serverchan)) {
				return (1);
			} else {
				x = 0;
				userchan++;
			}
		} else {
			temp[x++] = *userchan++;
		}
	}
	/* check last one */
	temp[x] = '\0';
	if (strcaseeq(temp, serverchan)) {
		return (1);
	}

	return (0);
}


/* check password for ops */
int check_pass(const char *userpass, const char *serverpass)
{

	/*
	   password set, password given        1 if match, 0 if fail
	   password set, no password given     0
	   no password set, password given       1
	   no password set, no password given  1
	 */

	if (userpass == NULL) {
		/* no password set, authentication ok */
		return (1);
	}

	if (userpass != NULL && serverpass == NULL) {
		return (0);
	}



	/* passwords equal - case sensitive */
	if (streq(userpass, serverpass)) {
		return (1);
	}

	return (0);
}

int free_list()
{
	struct userdata *userdata_curr, *userdata_tmp;

	userdata_curr = conf_userdata;
	conf_userdata = NULL;

	while (userdata_curr != NULL) {
		free(userdata_curr->usernick);
		free(userdata_curr->userip);
		free(userdata_curr->userchan);
		free(userdata_curr->userpass);

		userdata_tmp = userdata_curr;
		userdata_curr = userdata_curr->next;
		free(userdata_tmp);

	}

	return (0);
}

int insert_users(char *nick,
		 char *ip, char *status, char *channels, char *password)
{
	struct userdata *userdata_curr;

	int x = 0;
	char temp;

	if (conf_userdata == NULL) {
		conf_userdata = malloc(sizeof(struct userdata));
		conf_userdata->prev = NULL;
		userdata_curr = conf_userdata;
	} else {
		userdata_curr = conf_userdata;
		FindEndOfList(userdata_curr);
		userdata_curr->next = malloc(sizeof(struct userdata));
		userdata_curr->next->prev = userdata_curr;
		userdata_curr = userdata_curr->next;
	}
	userdata_curr->next = NULL;

	userdata_curr->userpass = NULL;

	if (password != NULL) {
		userdata_curr->userpass = strdup(password);
	}

	userdata_curr->usernick = strdup(nick);
	userdata_curr->userip = strdup(ip);
	userdata_curr->userchan = strdup(channels);

	userdata_curr->userstatus = atoi(status);

	while (userdata_curr->userip[x]) {
		temp = userdata_curr->userip[x];
		userdata_curr->userip[x] = tolower(temp);
		x++;
	}



	return (0);
}

struct commandlist *addcommand(void *module, struct commandlist *list,
			       const char *command, int (*function) (),
			       int access, int flags)
{
	struct commandlist *listptr;

	if (list == NULL) {
		list = malloc(sizeof(struct commandlist));
		list->prev = NULL;
		listptr = list;
	} else {
		listptr = list;
		FindEndOfList(listptr);
		listptr->next = malloc(sizeof(struct commandlist));
		listptr->next->prev = listptr;
		listptr = listptr->next;
	}
	listptr->next = NULL;
	listptr->cmd = strdup(command);
	listptr->function = function;
	listptr->module = module;
	listptr->access = access;
	listptr->flags = flags;
	return (list);
}

struct commandlist *delcommand(void *module, struct commandlist *list,
			       const char *command)
{
	struct commandlist *listptr;

	listptr = list;
	while (listptr != NULL) {
		if (listptr->module == module
		    && strcaseeq(command, listptr->cmd)) {
			UnlinkListItem(listptr, list);
			free(listptr->cmd) ;
			free(listptr) ;
			break;
		}
		listptr = listptr->next;
	}

	return (list);
}

struct commandlist *delcommandmodule(void *module, struct commandlist *list)
{
	struct commandlist *listptr;

	listptr = list;

	while (listptr != NULL) {
		if (listptr->module == module) {
			UnlinkListItem(listptr, list);
			free(listptr->cmd) ;
			free(listptr) ;
		}
		listptr = listptr->next;
	}

	return (list);
}
