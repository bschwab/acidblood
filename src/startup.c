#include "acidblood.h"
#include "acidfuncs.h"
#include "startup.h" 
#include "internal.h"
#include "extern.h"
#include "config.h"

/* put all commands you want loaded on startup here */

void startup() {
	totalbytes = 0;
	               
	printf("Acidblood %s\n", VERSION);
	init_conf() ;
	init_signals() ;
	init_networking() ;
	conf_userdata=NULL ;
	conf_channels=NULL ;
	init_usercommands() ;
	init_ctcp() ;
	init_srvcommands() ;
	init_log();
	init_serverinfo() ;
	init_numeric() ;
	init_timers() ;
	init_events() ;
	read_main_config() ;
	init_modules() ;
}
