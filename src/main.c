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

#include "acidblood.h"
#include "internal.h"
#include "acidfuncs.h"
#include "extern.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int UNUSED(argc), char * UNUSED(argv[]))
{
	if ((botinfo = malloc(sizeof(struct botstruct))) == NULL) {
		fprintf(stderr, "main: Malloc error!\n");
		acid_shutdown(-1);
	}
	
	time(&currtime);
	time(&botinfo->starttime);

	startup();

	if(botinfo->ver == NULL) {
		botinfo->ver = malloc(50) ;
		snprintf(botinfo->ver, 50, "Acidblood %s\n",VERSION) ;
	}

	/* load user data */
	if ((read_user_data()) < 0) {
		fprintf(stderr, "Error reading in user file.\n");
		acid_shutdown(-1);
	}
	
	background() ;

	while (1) {
		
		time(&currtime);

		CheckConfServers() ;
	
		check_connections();
		
		CheckTimers() ;
		
		ReapUserList();

	}
}
