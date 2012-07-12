/* Acidblood parsing routines */
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define CTCP_DELIM_CHAR '\001'

/* 
 * streq checks that the two 
 * provided strings are 
 * both non null and equal.
 *
 * This fucntion is much faster 
 * than strcmp and as an added 
 * bonus won't core if you pass 
 * it NULL 
 */
int streq(const char *str1, const char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return (0);

	if (*str1 == '\0' || *str2 == '\0')
		return (0);

	do {
		if (*str1 != *str2) {
			return (0);
		}
		str1++;
		str2++;
	} while (*str1 != '\0');

	if (*str2 == '\0') {
		return (1);
	}

	return (0);
}

/* 
 * strcaseeq checks that the two 
 * provided strings are 
 * both non null and equal ignoring case.
 *
 * This fucntion is much faster 
 * than strcasecmp and as an added 
 * bonus won't core if you pass 
 * it NULL 
 */
int strcaseeq(const char *str1, const char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return (0);

	if (*str1 == '\0' || *str2 == '\0')
		return (0);

	do {
		if (tolower(*str1) != tolower(*str2)) {
			return (0);
		}
		str1++;
		str2++;
	} while (*str1 != '\0');

	if (*str2 == '\0') {
		return (1);
	}

	return (0);

}

char GetCommandChar()
{
	return (botinfo->commandchar);
}

void safefree(char *fv)
{
	if (fv != NULL)
		free(fv);
}

void SplitInput(char *buf, struct inputstruct *ins)
{
	if (buf[0] == ':') {
		ins->prefix = strtok(buf, " ");
		ins->command = strtok(NULL, " ");
	} else {
		ins->prefix = (char *) NULL;
		ins->command = strtok(buf, " ");
	}
	ins->params = strtok(NULL, "\r");
}

void build_clientinfo(struct clientinfo *client, struct server_list *server, char *nick)
{
	client->server = server;
	client->nick = nick;
	client->ident = NULL;
	client->address = NULL;
}

void convert_clientinfo(struct socket_info *sinfo, char *input, struct clientinfo *client)
{

	strip_char_from_end(':', input);

	client->server = GetServerBySocket(sinfo);

	if (strchr(input, '!') != NULL) {
		client->nick = strtok(input, "!");
		client->ident = strtok(NULL, "@");
		client->address = strtok(NULL, "");
	} else {
		/* dont have username or ip */
		client->nick = input;
		client->ident = NULL;
		client->address = NULL;
	}
}

void strip_char_from_end(char strip, char *data)
{
	int len, i;

	/* 
	   This doesn't look nearly as cool as the old version
	   but it's a hell of a lot faster.

	   --Gerhard
	 */
	i = 0 ;
	
	if (data[0] == strip) {
		while(data[i] != '\0') {
			data[i] = data[i+1] ;
			i++ ;
		}
	}

	len = strlen(data) - 1;
	
	if (data[len] == strip) {
		data[len] = '\0';
	}
}

/* parse channels from the config file */
void parse_channels(char *line)
{
	char temp[100];
	char key[100];
	int x = 0;
	int y = 0;
	int keyflag = 0;
	int haskey = 0;

	while (*line != '\0') {

		/* beginning of a key */
		if (*line == '(') {
			keyflag = 1;
			haskey = 1;
			line++;
			continue;
		}

		/* ending of a key */
		if (*line == ')') {
			keyflag = 0;
			key[y] = '\0';
			line++;
			continue;
		}

		/* we are processing a key */
		if (keyflag) {
			key[y++] = *line;
			line++;
			continue;
		}

		/* end of a channel */
		if (*line == ',') {
			temp[x] = '\0';
			if (haskey == 1) {
				insert_channels(temp, key);
			} else {
				insert_channels(temp, NULL);
			}
			y = 0;
			key[y] = '\0';
			x = 0;
			haskey = 0;
		}


		else {
			temp[x++] = *line;
		}
		line++;
	}

	temp[x] = '\0';
	if (haskey == 1) {
		insert_channels(temp, key);
	} else {
		insert_channels(temp, NULL);
	}
}

void lowerstring(char *data)
{
	int cnt = 0;
	char tmp;
	if (data == NULL) {
		return;
	}

	while (data[cnt] != '\0') {
		tmp = data[cnt];
		data[cnt] = tolower(tmp);
		cnt++;
	}
}
