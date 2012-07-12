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

struct chusers {
	struct chusers *prev;
	char *nick;
	char *channel;
	struct chusers *next;
};

struct chusers *seen_chusers;

static void seen_join(int, struct socket_info *, char *, char *, char *);
static void seen_leave(int, struct socket_info *, char *, char *, char *);
static void seen_nick(int, struct socket_info *, char *, char *, char *);
static void clearmem(void) ;
static int seen_cmd(const char *replyto, struct userlist *user, char *data);
static char seenpath[200];

static int badnickstart(char s) 
{
	return( (s == '~')|| (s == '.') || ( s == '/') ) ;
}

int module_init(void *module)
{
	DIR *dpt;

	snprintf(seenpath, 200, "%s/seen", DATADIR);

	if ((dpt = opendir(seenpath)) == NULL) {
		if (mkdir(seenpath, 0700) == -1) {
			info_log("seen: unable to open working directory: %s\n", seenpath);
			return (-1);
		}
	}
	closedir(dpt);

	addrwusrcommand(module, "SEEN", seen_cmd, 99);
	addtrigger(module, EV_JOIN, seen_join);
	addtrigger(module, EV_HERE, seen_join);
	addtrigger(module, EV_PART, seen_leave);
	addtrigger(module, EV_QUIT, seen_leave);
	addtrigger(module, EV_NICK, seen_nick);
	return (0);
}

int module_shutdown(void *module)
{
	deltrigger(module, EV_JOIN);
	deltrigger(module, EV_PART);
	deltrigger(module, EV_QUIT);
	deltrigger(module, EV_NICK);
	deltrigger(module, EV_HERE);
	delusrcommand(module, "seen");
	clearmem() ;
	return (0);
}

static void logseen(char *nick, char *channel)
{
	FILE *seenfile;
	char timestr[100], *filename;

	if (channel == NULL) {
		return;
	}

	lowerstring(channel);

	if (chdir(seenpath) == -1) {
		info_log("seen: unable to change to directory %s\n", seenpath);
		return;
	}

        /*
         *  Normally not possible to have a bad nick here but we 
         *  never know who modded the irc server to allow what options.
         *  We block characters that allow the user to change directory.
         *  Doubly important since despite all logic some people seem to 
         *  run acidblood as root.
         *
         */
	if (badnickstart(*nick)) {
		return ;
	} 
	filename = strdup(nick);
	lowerstring(filename);
	seenfile = fopen(filename, "w");

	if (seenfile == NULL) {
		info_log("unable to open \"%s\"\n", filename);
		free(filename);
		return;
	}

	free(filename);

	time(&currtime);
	strftime(timestr, sizeof(timestr), "%b %d %H:%M %Y", localtime(&currtime));

	fprintf(seenfile, "%s was last seen in %s on %s\n", nick, channel, timestr);

	fclose(seenfile);
}

static struct chusers *seen_find_inchannel(const char *nick, const char *channel)
{
	struct chusers *curusers;

	curusers = seen_chusers;

	while (curusers != NULL) {
		if (strcaseeq(nick, curusers->nick)
		    && (channel == NULL || strcaseeq(channel, curusers->channel))) {
			return (curusers);
		}
		curusers = curusers->next;
	}
	return (NULL);
}

static int seen_is_inchannel(const char *nick, const char *channel)
{
	if (seen_find_inchannel(nick, channel) == NULL) {
		return (0);
	}
	return (1);
}

static void seen_user_arrive(const char *nick, const char *channel)
{
	struct chusers *curusers, *tmpusers;
	curusers = seen_find_inchannel(nick, channel);

	if (curusers != NULL) {
		return;
	}

	curusers = malloc(sizeof(struct chusers));
	curusers->nick = strdup(nick);
	curusers->channel = strdup(channel);

	LinkToList(curusers, tmpusers, seen_chusers);
}

static void seen_user_leave(char *nick, char *channel)
{
	struct chusers *curusers;

	curusers = seen_find_inchannel(nick, channel);

	while (curusers != NULL) {
		logseen(nick, curusers->channel);

		UnlinkListItem(curusers, seen_chusers);
		free(curusers->nick);
		free(curusers->channel);
		free(curusers);

		curusers = seen_find_inchannel(nick, channel);
	}
}

static void seen_user_rename(char *nick, char *newnick)
{
	struct chusers *curusers;

	curusers = seen_find_inchannel(nick, NULL);

	while (curusers != NULL) {
		logseen(curusers->nick, curusers->channel);
		free(curusers->nick);
		curusers->nick = strdup(newnick);

		curusers = seen_find_inchannel(nick, NULL);
	}
}

static void seen_join(/*@unused@*/__attribute__((unused)) int event, /*@unused@*/__attribute__((unused)) struct socket_info *sinfo, char *nick, char *dest, /*@unused@*/__attribute__((unused)) char *info)
{
	seen_user_arrive(nick, dest);
}

static void seen_leave(/*@unused@*/__attribute__((unused)) int event, /*@unused@*/__attribute__((unused)) struct socket_info *sinfo, char *nick, /*@unused@*/__attribute__((unused)) char *dest, /*@unused@*/__attribute__((unused)) char *info)
{
	seen_user_leave(nick, dest);
}

static void seen_nick(/*@unused@*/__attribute__((unused)) int event, /*@unused@*/__attribute__((unused)) struct socket_info *sinfo, char *nick, /*@unused@*/__attribute__((unused)) char *dest, /*@unused@*/__attribute__((unused)) char *info)
{

	/*
	 * abort if the user only switched case 
	 */
	if (strcaseeq(nick, info)) {
		return;
	}
	seen_user_rename(nick, info);
}

static int seen_cmd(const char *replyto, struct userlist *user, char *data)
{
	FILE *seenfile;
	char newpath[200], line[200];
	char *targnick ;

	if (replyto[0] != '#') {
		return (0);
	}
	
	targnick = strtok(data, " ") ;
	lowerstring(targnick) ;
	
        /*
         * Make sure no one is playing games with the nick 
         * and adding characters that would allow changing 
         * the directory.
         * Doubly important since despite all logic some people seem to 
         * run acidblood as root.
         *
         */
	if (badnickstart(*targnick)) {
		msgreply(user, replyto, "invalid nick\n") ;
		return (0) ;
	}
	
	if (seen_is_inchannel(targnick, replyto)) {
		msgreply(user, replyto, "%s is here\n", targnick);
		return (0);
	}

	if (chdir(seenpath) == -1) {
		info_log("seen: unable to change to directory %s\n", newpath);
		return (0);
	}
	seenfile = fopen(targnick, "r");

	if (seenfile == NULL) {
		msgreply(user, replyto, "I have never met %s\n", targnick);
		return (0);
	}

	fgets(line, 512, seenfile);
	msgreply(user, replyto, "%s\n", line);

	fclose(seenfile);
	return (0);
}

static void clearmem()
{
	struct chusers *curusers ;

	while (seen_chusers != NULL) {

		curusers=seen_chusers ;
		seen_chusers=curusers->next ;
	
		free(curusers->nick);
		free(curusers->channel);
	}
}
