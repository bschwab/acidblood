#include <stdlib.h>

#include "acidblood.h"
#include "startup.h"
#include "extern.h"
#include "acidfuncs.h"
#include "list.h"


/* 
 * timers.. Use these sparingly  
 * Note that these are not exact and 
 * may be several seconds late.  
 */

struct timerstruct {
	struct timerstruct *prev ;
	time_t deadline ;
	void (*function)() ;
	void *module ;
	int delete ;
	struct timerstruct *next ;
} ;

static struct timerstruct *timers ;

void addtimer(void *module, time_t deadline, void (*function)()) 
{
	struct timerstruct *timers_curr, *timers_tmp ;
	
	timers_tmp=malloc(sizeof(struct timerstruct)) ;

	timers_tmp->module=module ;
	timers_tmp->deadline=deadline ;
	timers_tmp->function=function ;
	timers_tmp->delete=0 ;
	
	if (timers == NULL) {
		timers = timers_tmp ;
		timers_tmp->prev = NULL ;
		timers_tmp->next = NULL ;
		return ;
	} 
	
	timers_curr = timers ;
	
	while (timers_curr->next != NULL && timers_curr->next->deadline <= deadline ) {
		timers_curr = timers_curr-> next ;
	}
	
	timers_tmp->prev = timers_curr ;
	timers_tmp->next =  timers_curr->next ;
	
	if (timers_curr->next != NULL) {
		timers_curr->next->prev = timers_tmp ;
	}
	 
	timers_curr->next=timers_tmp ;
	
}

void schedulein(void *module, int delay, void (*function)()) 
{
	time(&currtime);
	addtimer(module, currtime+delay,function) ;	
}
 
void deltimer(void *module,time_t deadline) 
{
	struct timerstruct *timers_curr ;
	
	timers_curr=timers ;
	
	while(timers_curr!=NULL) {
		if(timers_curr->module==module
		   &&timers_curr->deadline==deadline) {
			UnlinkListItem(timers_curr,timers) ;
			free(timers_curr) ;
			return ;   
		}
		timers_curr=timers_curr->next ;
	}
}

void deltimermodule(void *module) 
{
	struct timerstruct *timers_curr, *next ;
	
	timers_curr=timers ;
	
	while(timers_curr!=NULL) {
		next=timers_curr->next ;
		if(timers_curr->module==module){
			UnlinkListItem(timers_curr,timers) ;
			free(timers_curr) ;
		}
		
		timers_curr=next ;
	}
}

void CheckTimers() 
{
	struct timerstruct *timers_curr, *next ;
	time(&currtime);
	
	timers_curr=timers ;
	
	while(timers_curr != NULL && timers_curr->deadline <= currtime) {
		next=timers_curr->next ;
		
		timers_curr->function() ;
		UnlinkListItem(timers_curr,timers) ;
		free(timers_curr) ;
		
		timers_curr=next ;
	}
	
}

void init_timers() 
{
	timers=NULL ;
}
