#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "startup.h"
#include "acidblood.h"
#include "acidfuncs.h"
#include "tracking.h"
#include "list.h"
#include "config.h"
#include "extern.h"

struct modstruct {
	struct modstruct *prev ;
	char *name ;
	void *handle ;	
	struct modstruct *next ;
} ;

static void autoloadmods(void) ;

#ifndef 	RTLD_NOW
#warning OS does not implement RTLD_NOW using RTLD_LAZY instead
#define 	RTLD_NOW 	RTLD_LAZY
#endif

static struct modstruct *modules ;

void init_modules() {
	modules=NULL ;	
	autoloadmods() ;
}

static void unlinkmod(struct modstruct *module) {
#ifdef MODULES
	void (*modshutdown)() ;
	
	//Ugly? why yes it is.. the extra void cast is ther to shut GCC up.
	*(void **)((void *)&modshutdown)=dlsym(module->handle, "module_shutdown") ;
	modshutdown(module->handle) ;
	dlclose(module->handle) ;
	UnlinkListItem(module,modules) ;

	delnumericmodule(module->handle) ;
	delctcpmodule(module->handle) ;
	delusrcommandmodule(module->handle) ;
	delservercommandmodule(module->handle) ;
	deltimermodule(module->handle)  ;
	deltriggermodule(module->handle) ;
	delnetworkmodule(module->handle) ;
#endif
}

static int initmodule(const char *modname, char *modpath) {
#ifdef MODULES
	struct modstruct *modcurr ;
	int (*modinit)() ;
	void *handle ;
	int ret ;
	
	info_log("loading module: %s\n", modname) ;

	handle = dlopen(modpath, RTLD_NOW);

	if(!handle) {
		info_log("%s\n",dlerror()) ;
		return(FALSE) ;
	}
	
	if(modules==NULL) {
		modules=malloc(sizeof(struct modstruct)) ;
		modules->prev=NULL ;
		modcurr=modules ;
	} else {
		modcurr=modules ;
		FindEndOfList(modcurr) ;
		modcurr->next=malloc(sizeof(struct modstruct)) ;
		modcurr->next->prev=modcurr ;
		modcurr=modcurr->next ;
	}
	modcurr->next=NULL ;
	modcurr->name=strdup(modname) ;
	modcurr->handle = handle ;
	
	// The extra void cast is there to shut GCC up.
	*(void **)((void *)&modinit)=dlsym(modcurr->handle, "module_init");
	ret=modinit(modcurr->handle) ;

	if (ret < 0 ) {
		info_log("Module init %s failed: %i\n", modname, ret) ;
		unlinkmod(modcurr) ;
	}
	return(ret) ;
#else
	return(0) ;
#endif
}

static void autoloadmods() {
#ifdef MODULES
	char path[256] ;
	struct dirent *entry ;
	struct stat statbuf ;
	DIR *dpt ;

	if((dpt=opendir(MODPATH)) ==NULL ) {
        	info_log("Cannot open module directory %s\n",MODPATH) ;
        	return ;
	}

	while((entry = readdir(dpt)) !=NULL) {
		if(entry->d_name[0]!='.') {
			stat(entry->d_name,&statbuf) ;
			if(!S_ISDIR(statbuf.st_mode)) {
				snprintf(path,250,"%s/%s",MODPATH, entry->d_name) ;
				initmodule(entry->d_name, path) ;
			}
		}
	}
	closedir(dpt) ;
#endif		        
}

int loadmodule(const char *modname) {
#ifdef MODULES
	char path[256] ;
	
	/* remove any old copies of the module before loading the new */
	removemodule(modname) ;
	
	/* check for an absolute path */
	if(*modname == '/' || *modname == '.') {
		snprintf(path,250,"%s", modname) ;
	} else {
		snprintf(path,250,"%s/%s",MODPATH, modname) ;
	}
	return(initmodule(modname,path)) ;
#else 
	return(0) ;
#endif
}	

int removemodule(const char *modname) {
#ifdef MODULES
	struct modstruct *modcurr ;
	
	modcurr=modules ;
	
	while(modcurr!=NULL) {
		if(strcaseeq(modname, modcurr->name)) {
			unlinkmod(modcurr) ;
			return(TRUE) ;
		}
		
		modcurr=modcurr->next ;
		
	}
	return(FALSE) ;
#else
	return(TRUE) ;
#endif	                
}

void listmods(const char *replyto, struct userlist *user) {
#ifdef MODULES
	struct modstruct *modcurr ;
	
	modcurr=modules ;
	
	while(modcurr!=NULL) {
		msgreply(user, replyto, "module: %s", modcurr->name) ; 
		modcurr=modcurr->next ;
	}
	msgreply(user, replyto,"End of Modules") ;
#endif
}
