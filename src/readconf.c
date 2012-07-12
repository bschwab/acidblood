#include <stdlib.h>
#include <ctype.h>

#include "startup.h"
#include "includes.h"
#include "acidblood.h"
#include "network.h"
#include "extern.h"
#include "config.h"
#include "acidfuncs.h"

struct cnfs *addconf(struct cnfs *configs, const char *name, void (*function)(char *))
{
	struct cnfs *newconf;
	struct cnfs *tmpconf;

	newconf = malloc(sizeof(struct cnfs));

	newconf->command = strdup(name);
	newconf->function = function;
	newconf->next = NULL;

	if (configs == NULL) {
		return  (newconf) ;
	}

	tmpconf = configs;

	while (tmpconf->next != NULL) {
		tmpconf = tmpconf->next;
	}

	tmpconf->next = newconf;
	return(configs) ;
}

struct cnfs *clearconf(struct cnfs *configs) 
{
	struct cnfs *tmpconf;

	if(configs==NULL) 
		return(NULL) ;

	tmpconf=configs ;

	while (configs != NULL) {
		tmpconf=configs->next ;
		free(configs->command) ;
		free(configs) ;
		configs = tmpconf;
	}
	
	return(configs) ;
}


static void execconf(struct cnfs *configs, char *command, char *data) 
{
	struct cnfs *tmpconf;

	if(configs==NULL) 
		return ;

	tmpconf=configs ;

	while (tmpconf != NULL) {
		if (strcaseeq(command, tmpconf->command)) {
			tmpconf->function(data) ;
			return ;
		}
		tmpconf = tmpconf->next;
	}
}

static char *cleanup(char *data)
{
	int len;

	if (data == NULL)
		return (NULL);

	len = strlen(data) -1 ;

	while (len != 0) {
		if (*data != ' ')
			break;

		len--;
		data++;
	}

	if (len > 1 && (data[len] == '\n' || data[len] == '\r')) {
		data[len] = '\0';
		len--;
	}

	if (len > 1 && (data[len] == '\n' || data[len] == '\r')) {
		data[len] = '\0';
		len--;
	}

	if (len < 1)
		return (NULL);

	return (data);
}


void read_config(struct cnfs *cnnflist, const char *configfile)
{
	FILE *fp_config;
	char *tline,  *line, *command, *data;
	struct cnfs *confcmds ;
	
	configfile = CONFIG ;
	confcmds = cnnflist ;
	if ((fp_config = fopen(configfile, "r")) == NULL) {
		fprintf(stderr, "Cannot open config file %s!\n", CONFIG);
		acid_shutdown(-1);
	}

	if ((tline = malloc(1000)) == NULL) {
		fprintf(stderr, "read_config: Malloc error!\n");
		acid_shutdown(-1);
	}

	line=tline ;
	
	while ((fgets(line, 1000, fp_config)) != NULL) {
		if (isalpha(*line)) {
			command = strtok(line, "=");
			data = strtok(NULL, "");

			if (data != NULL && strlen(data) <= 1)
				data = NULL;

			command = cleanup(command);
			data = cleanup(data);
		
			execconf(confcmds,command,data) ;
		}
		line=tline ;
	}

	free(tline);
	fclose(fp_config);
}
