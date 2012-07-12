/* acidblood header file */

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

#define VERSION "1.2.21"

#include <stdio.h>
#include <time.h>

#define TRUE  1
#define FALSE 0

#define SUCCESS 0
#define FAILURE -1

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) CANTUSE_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

struct botstruct *botinfo;

time_t currtime;


struct clientinfo {
	char *nick;
	char *ident;
	char *address;
	struct server_list *server;
};

struct inputstruct {
	char *prefix;
	char *command;
	char *params;
};
                        

typedef struct socket_info socket_info ;

struct socket_info {
	int fd ;
	int flags ;
	int last_attempt ;
	int buffpos ;
	int timeout ;
	int keepalive ;
	time_t lastrcvpacket ;
	time_t lastsndpacket ;
	time_t pingtimeout ;
	char buff[1500] ;
	void *module ;
	void (*handler)(int, socket_info *) ;
	void (*idle) (socket_info *) ;
};
                                


// fixme: remove later
struct socket_info socket_pool[200] ;
                                
typedef struct clientinfo clientinfo;

struct userdata *conf_userdata;

/* massdeop list */
struct mddata {
	char *nick;
	struct mddata *next;
};

struct channels *conf_channels;

struct commandlist {
	struct commandlist *prev;
	char *cmd;
	int (*function) ();
	void *module;
	int access;
	int flags;
	struct commandlist *next;
};

struct userlist {
	struct userlist *prev;
	struct auser *userinfo;
	double modes;
	int access;
	time_t lastctcp ;
	time_t lastcmd ;
	struct userlist *next;
};

struct cnfs {
	char *command;
	void (*function) (char *);   
	struct cnfs *next;   
};
                        
struct commandlist *usercommandlist;
struct commandlist *ctcplist;
struct commandlist *servercommandlist;
struct commandlist *dcclist ;

/* Global variables */
struct mddata *curr2;		/* pointers for 2nd linked list of users for massdeops */
struct mddata *top2;
struct mddata *prev2;

char *protect;			/* protected user from massdeops */
long totalbytes;
