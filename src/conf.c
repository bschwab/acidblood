#include <stdlib.h>

#include "includes.h"
#include "internal.h"
#include "acidblood.h"
#include "acidfuncs.h"
#include "network.h"
#include "extern.h"
#include "startup.h"
#include "config.h"

static struct cnfs *confs = NULL;

static void addmainconf(const char *name, void (*function)(char *))
{
	confs=addconf(confs,name,function) ;
}

static void conf_user(char *data) 
{
	char *tmp ;
	tmp=strtok(data,":") ;
	
	if(tmp==NULL) {
		info_log("conf_user: No username supplied\n") ;
		return ;  
	}
	safefree(botinfo->user) ;
	
	botinfo->user=strdup(data);
	
	debug_log("Bot username=%s\n", botinfo->user);
	
} 

static void conf_nick(char *data) 
{
	safefree(botinfo->nick) ;
	botinfo->nick=strdup(data);
	
	debug_log("Bot nick=%s\n", botinfo->nick);
}

static void conf_alt(char *data) 
{
	safefree(botinfo->altnick) ;
	botinfo->altnick=strdup(data);
	debug_log("Bot altnick=%s\n", botinfo->altnick);
	                                          
}

static void conf_cmdchar(char *data) 
{
	botinfo->commandchar = data[0];
	debug_log("Bot char=%c\n",botinfo->commandchar) ;
}

static void conf_fullname(char *data) 
{
	safefree(botinfo->fname) ;
	botinfo->fname = strdup(data);
	
	debug_log("Bot fullname=%s\n",botinfo->fname);
}

static void conf_channel(char *data)
{
	parse_channels(data);
}

static void conf_srvpass(char *data)
{
	safefree(botinfo->srvpass) ;
	botinfo->srvpass = strdup(data);
	
	debug_log("Server pass=%s\n", botinfo->srvpass);
}

static void conf_server(char *data)
{
	safefree(botinfo->server) ;
	botinfo->server = strdup(data);
	
	if(botinfo->port!=0) {
		AddServerConf(data,botinfo->port,NULL) ;
	} else {
		AddServerConf(data,6667,NULL) ;
	}
	debug_log("Bot server=%s\n",botinfo->server);
}

static void conf_mode(char *data)
{
	safefree(botinfo->mode) ;
	botinfo->mode = strdup(data) ;
	debug_log("Mode=%s\n", botinfo->mode);
}

static void conf_awaymsg(char *data)
{
	safefree(botinfo->awaymsg) ;
	botinfo->awaymsg = strdup(data) ;
}

static void conf_ctcp(char *data)
{
	if (strcaseeq("ON", data)) {
		botinfo->ctcp = 1;
	}
}

static void conf_version(char *data)
{
	safefree(botinfo->ver) ;
	botinfo->ver = strdup(data) ;
}

static void conf_port(char *data)
{
	botinfo->port = atoi(data);
	
}

static void conf_nserv(char *data)
{
	if (strcaseeq("ON", data)) {
		botinfo->ns = 1;
	} else {
		botinfo->ns = 0 ;
	}
	debug_log("NickServ routines=%s\n", data) ;
}

static void conf_nspass(char *data)
{
	safefree(botinfo->nspass) ;
	botinfo->nspass = strdup(data) ;
	debug_log("Nickserv Pass=%s\n", botinfo->nspass);
}

static void conf_autoop(char *data)
{
	if (strcaseeq(data, "ON")) {
		botinfo->autoop = 1;
		debug_log("Auto-op is ON\n");
	} else {
		botinfo->autoop = 0;
		debug_log("Auto-op is Off\n");
	}
}

static void conf_reqpass(char *data)
{
	if (strcaseeq(data, "ON")) {
		botinfo->reqpass = 1;
	} else {
		botinfo->reqpass = 0;
	}
}

static void conf_keepalive(char *data)
{
	botinfo->keepalive = atoi(data);
	debug_log("Keepalive is %d sec(s)\n", botinfo->keepalive);
}

static void conf_timeout(char *data)
{
	botinfo->timeout = atoi(data);
	debug_log("Timeout is %d min(s)\n", botinfo->timeout);
}

static void conf_maxtries(char *data)
{
	botinfo->maxtries= atoi(data);
	debug_log("maxtries: %i\n",botinfo->maxtries) ;
	
}

static void conf_cmddelay(char *data)
{
	botinfo->cmddelay= atoi(data);
	debug_log("cmddelay: %i\n",botinfo->cmddelay) ;
	
}

static void conf_ctcpdelay(char *data)
{
	botinfo->ctcpdelay= atoi(data);
	debug_log("ctcpdelay: %i\n",botinfo->ctcpdelay) ;
	
}

static void conf_oplevel(char *data)
{
	botinfo->oplevel= atoi(data);
	debug_log("oplevel: %i\n",botinfo->oplevel) ;
	
}


static void conf_hoplevel(char *data)
{
	botinfo->hoplevel= atoi(data);
	debug_log("hoplevel: %i\n",botinfo->hoplevel) ;
	
}

static void conf_voicelevel(char *data)
{
	botinfo->voicelevel= atoi(data);
	debug_log("voicelevel: %i\n", botinfo->voicelevel) ;
	
}

static void conf_floodexempt(char *data)
{
	botinfo->floodexempt= atoi(data);
	debug_log("exemptlevel: %i\n", botinfo->floodexempt) ;
	
}

void init_conf() 
{
	addmainconf("USER", conf_user) ;
	addmainconf("NICK", conf_nick) ;
	addmainconf("ALT", conf_alt) ;
	addmainconf("COMMANDCHAR", conf_cmdchar) ;
	addmainconf("FULLNAME", conf_fullname) ;
	addmainconf("CHANNELS", conf_channel) ;
	addmainconf("SRVPASS", conf_srvpass) ;
	addmainconf("SERVER", conf_server) ;
	addmainconf("MODE", conf_mode ) ;
	addmainconf("AWAYMSG", conf_awaymsg ) ;
	addmainconf("CTCP", conf_ctcp ) ;
	addmainconf("PORT", conf_port ) ;
	addmainconf("VERSION", conf_version ) ;
	addmainconf("NSERV", conf_nserv ) ;
	addmainconf("NSPASS", conf_nspass ) ;
	addmainconf("AUTOOP", conf_autoop ) ;
	addmainconf("REQPASS", conf_reqpass ) ;
	addmainconf("KEEPALIVE", conf_keepalive ) ;
	addmainconf("TIMEOUT", conf_timeout ) ;
	addmainconf("MAXTRIES", conf_maxtries ) ;
	addmainconf("CMDDELAY", conf_cmddelay) ;
	addmainconf("CTCPDELAY", conf_ctcpdelay) ;
	addmainconf("OPLEVEL", conf_oplevel) ;
	addmainconf("HOPLEVEL", conf_hoplevel) ;
	addmainconf("VOICLEVEL", conf_voicelevel) ;
	addmainconf("FLOODEXEMPT", conf_floodexempt) ;
}

void read_main_config()
{
	read_config(confs,CONFIG) ;
}