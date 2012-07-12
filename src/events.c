#include <stdlib.h>

#include "startup.h"
#include "acidblood.h"
#include "acidfuncs.h"
#include "list.h"
#include "events.h"
#include "internal.h"

struct triggerstruct {
	struct triggerstruct *prev ;
	int event ;
	void (*function)() ;
	void *module ;
	struct triggerstruct *next ;
} ;

struct triggerstruct *bottriggers ;

void addtrigger(void *module, int event, void (*function)()) 
{
	struct triggerstruct *triggers_curr, *triggers_tmp ;
	
	triggers_tmp=malloc(sizeof(struct triggerstruct)) ;
	
	LinkToList(triggers_tmp,triggers_curr,bottriggers) ;
	
	triggers_curr->module=module ;
	triggers_curr->event=event ;
	triggers_curr->function=function ;
}

void deltrigger(void *module, int event) 
{
	struct triggerstruct *triggers_curr ;
	
	triggers_curr=bottriggers ;
	
	while(triggers_curr!=NULL) {
		if(triggers_curr->module==module
		   && triggers_curr->event==event) {
			UnlinkListItem(triggers_curr,bottriggers) ;
			free(triggers_curr) ;
			return ;
		} 
		
		triggers_curr=triggers_curr->next ;
	}	
}

void deltriggermodule(void *module) 
{
	struct triggerstruct *triggers_curr ;
	
	triggers_curr=bottriggers ;
	
	while(triggers_curr!=NULL) {
		if(triggers_curr->module==module) {
			UnlinkListItem(triggers_curr,bottriggers) ;
			free(triggers_curr) ;
		} 
		
		triggers_curr=triggers_curr->next ;
	}	
}

void botevent(int event,struct socket_info *sinfo, const char *nick, const char *dest, const char *info)
{
	struct triggerstruct *triggers_curr ;
	
	triggers_curr=bottriggers ;
	
	while(triggers_curr!=NULL) {
		if(triggers_curr->event==event) {
			triggers_curr->function(event,sinfo,nick,dest,info) ;
		}

		triggers_curr=triggers_curr->next ;
	}
}
 
void init_events() 
{
	bottriggers=NULL ;
}
