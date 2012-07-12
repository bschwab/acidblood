#include <signal.h>

#include "acidblood.h"
#include "internal.h"
#include "extern.h"
#include "acidfuncs.h"
#include "startup.h"

/* signal handler */
static void handler(int UNUSED(sig))
{
	acid_shutdown(0);
}

static void handle_hup(int sig)
{
	acid_reload("Got HUP signal. Reloading.\n") ;
	signal(sig,handle_hup) ;
}

void init_signals() 
{
	signal(SIGINT,handler);
	signal(SIGHUP,handle_hup) ;
	signal(SIGQUIT,handler);
	signal(SIGTERM,handler);
	
	signal(SIGPIPE,SIG_IGN) ;	
}
