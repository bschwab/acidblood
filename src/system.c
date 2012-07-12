#include "acidblood.h"
#include "internal.h"
#include "acidfuncs.h"
#include "extern.h"
#include "config.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void acid_shutdown(int errorlevel)
{
	info_log("shutting down\n") ;
	close_log();
	exit(errorlevel);
}

void acid_reload(const char *reason)
{
	close_log();
	init_log() ;
	info_log("Reload: %s\n",reason) ;
	read_user_data();
}

void background() 
{
	FILE *fp_pid;
	pid_t pid, sid ;
	
	if ((pid = fork())==-1) {
		printf("shutting down: unable to fork\n") ;
		info_log("shutting down: unable to fork\n") ;
		acid_shutdown(-1);
	}
	
	/* return if we are the parent */
	if(pid!=0) {
		exit(0) ;
	}
	
	if ((fp_pid = fopen(PIDFILE, "w")) == NULL) {
		info_log("Cannot open pid file.\n");
		fclose(fp_pid);
		acid_shutdown(-1);
	}	
	
	fprintf(fp_pid, "%d", getpid());
	fclose(fp_pid);	
	
	sid=setsid() ;
	
	if (sid == -1) {
		info_log("unable to set session id\n") ;
		acid_shutdown(-1);
	}
	
	close(STDIN_FILENO) ;
	close(STDOUT_FILENO) ;
        close(STDERR_FILENO) ; 
}
