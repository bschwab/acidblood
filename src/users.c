#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "acidblood.h"
#include "acidfuncs.h"
#include "internal.h"
#include "extern.h"
#include "config.h"
#include "tracking.h"

char *unick(struct userlist *user)
{
	return(user->userinfo->nick) ;
}

struct socket_info *usinfo(struct userlist *user)
{
	return(user->userinfo->server->sinfo) ;
}

/* read in user file */
int read_user_data()
{
	FILE *fp_users;
	char *data;
	char *servernick;
	char *serverip;
	char *statustemp;
	char *channels;
	char *password;

	if (free_list() < 0) {
		return (-1);
	}

	if ((data = malloc(1000)) == NULL) {
		fprintf(stderr, "read_user_data: Malloc Error!\n");
		return (-1);
	}

	if ((fp_users = fopen(USERS, "r")) == NULL) {
		return (-1);
	}

	while ((fgets(data, 1000, fp_users)) != NULL) {
		if (*data != '#' && (strlen(data) > 1)) {
		/* subcube:63.197.85.162:0:all:testpass */

			servernick = strtok(data, ":");
			serverip = strtok(NULL, ":");
			statustemp = strtok(NULL, ":");
			channels = strtok(NULL, ":");
			password = strtok(NULL, "\n");

			strip_char_from_end('\n', channels);

			if ((insert_users
			     (servernick, serverip, statustemp, channels,
			      password)) < 0) {
				return (-1);
			}
		}
	}

	fclose(fp_users);
	return (0);
}
