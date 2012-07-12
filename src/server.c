#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "acidblood.h"
#include "acidfuncs.h"
#include "internal.h"
#include "network.h"
#include "extern.h"
#include "tracking.h"
#include "startup.h"
#include "list.h"

struct servercstruct {
	struct servercstruct *prev ;
	char *address ;
	int port ;
	char *password ;
	int  conatt ;
	time_t nexttry ;
	int reap ;

	struct socket_info *sinfo ;
	struct server_list *server ;
	struct servercstruct *next ;	
} ;

static struct servercstruct *serverclist ;
 
void init_serverinfo() 
{
	serverclist=NULL ;
}

static void server_action(int action, struct socket_info *sinfo)
{
	if (action == NET_READ) {
		if (getline_server(sinfo) == 1)
			parse_server(sinfo);
		return;
	}

	if (action == NET_WRITE) {
		if (IsConnecting(sinfo)) {
		        NotConnecting(sinfo);
		        SetConnected(sinfo);
		        NotWriteable(sinfo) ;
		                                                                
			register_to_server(sinfo);
		}
		return;
	}
		
}

void complete_connect(struct socket_info *sinfo)
{
	struct server_list *server ;
	
	SetRegistered(sinfo) ;
	
	/* if an away message exists, set away */
	if (botinfo->awaymsg != NULL) {
		irc_sprintf(sinfo, "AWAY:%s \n", botinfo->awaymsg);
	}
	
	server=GetServerBySocket(sinfo) ;
	sinfo->keepalive = botinfo->keepalive ;
	sinfo->idle = pingonidle ;
	/* send mode information */

	irc_sprintf(sinfo,"MODE %s %s \n", server->nick, botinfo->mode);
	if (botinfo->ns == 1) {
	} else {
		join_channels(sinfo);
	}

}

int register_to_server(struct socket_info *sinfo)
{
	struct server_list *server ;
	server=GetServerBySocket(sinfo) ;
	
	if (botinfo->srvpass) {
		irc_sprintf(sinfo, "PASS %s \n", botinfo->srvpass);
	}

	/* user username hostname(ignored) servername(ignored) :realname */
	irc_sprintf(sinfo,"USER %s %s %s :%s \n",
		    botinfo->user, botinfo->server,
		    botinfo->mode, botinfo->fname);
	irc_sprintf(sinfo,"NICK %s \n", server->nick);
	return (0);
}

void AddServerConf(char *address, int port, char *password) {
	struct servercstruct *slist, *stmp ;
	stmp=malloc(sizeof(struct servercstruct)) ;
	
	
	LinkToList(stmp,slist,serverclist) ;

	slist->address=strdup(address) ;
	
	if(password!=NULL)
		slist->password=strdup(password) ;
	
	slist->port=port ;
	
	slist->sinfo=GetNewSocket(server_action) ;
	
	SetIrcProtocol(slist->sinfo) ;
	slist->nexttry=0  ;
	                
	if (slist->sinfo!=NULL)
		return ;

	UnlinkListItem(slist,serverclist) ;
	free(slist->address) ;
	free(slist->password) ;
	free(slist) ;		

}

static int CalcConDelay(int attempt) {
	/* no connects less than 10 seconds appart */
	if(attempt <= 3)
		return(10) ;
		
	/* exponentially increase the delay to avoid flooding servers*/
	return(attempt*attempt) ;		
}

static void CheckMissing(struct servercstruct *server) {
	int delay ;
	
	if(!IsConnecting(server->sinfo)&&(server->nexttry<=currtime) ) {
		if(server->server!=NULL) {
			DelServer(server->server) ;
			server->server=NULL ;
		}
		if(botinfo->maxtries != 0 && server->conatt >= botinfo->maxtries) {
			NotConnectable(server->sinfo) ;
			return ;
		}
			
		server->conatt++ ;
		delay=CalcConDelay(server->conatt) ;
		server->nexttry=currtime+delay;
		info_log("Connecting to %s attempt %i next attempt in %i seconds\n",server->address, server->conatt, delay) ; 
		AddServer(server->sinfo, server->address, botinfo->nick);

		/* convert botinfo->timout (minutes) to sinfo->timeout (seconds) */
		server->sinfo->timeout=botinfo->timeout * 60 ;
		disconnectonidle(server->sinfo) ; 
		
		connect_to_host(server->sinfo,server->address,server->port) ;
	}
	
}

static void ReapConfServers() {
	
	struct servercstruct *slist, *tnext ;
	
	slist=serverclist ;
	
	while(slist!=NULL) {
		tnext=slist->next ;
		
		if(slist->reap) {		
			UnlinkListItem(slist,serverclist) ;
			FreeSocket(slist->sinfo) ;
			free(slist->address) ;
			free(slist->password) ;
			free(slist) ;		
		}	
		slist=tnext ;
	}

}

void CheckConfServers() {
	struct servercstruct *slist ;
	
	ReapConfServers() ;
	
	slist=serverclist ;
	while(slist!=NULL) {
		if(!IsConnected(slist->sinfo)) { 
			CheckMissing(slist) ;
		} 
		
		if(IsRegistered(slist->sinfo) && slist->conatt != 0) {
			slist->conatt = 0 ;
		}
		
		if(IsNotConnectable(slist->sinfo))
			slist->reap=1 ;
		
		slist=slist->next ;
	}
}

int parse_server (struct socket_info *sinfo)
{
	struct inputstruct is ;
	
	SplitInput(sinfo->buff,&is) ;
	
	if(isalpha((int)is.command[1])) {
		return(handle_server_command(sinfo,&is));
	}
            
	return(handle_numeric(sinfo,&is)) ;
              
	return(0) ;
}
