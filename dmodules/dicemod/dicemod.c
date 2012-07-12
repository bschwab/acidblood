#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "acidblood.h"
#include "acidfuncs.h"
#include "extern.h"
#include "tracking.h"

typedef struct {
	int d1;
	int d2;
	int result;
} diereturn;

int dicemod_roll(const char *, struct userlist *, const char *);
int dicemod_attack(const char *, struct userlist *, char *);

int module_init(void *module)
{
	addusrcommand(module, "roll", dicemod_roll, 99);
	addrwusrcommand(module, "attack", dicemod_attack, 99);
	time(&currtime);
	srand(currtime);
	
	return(0) ;
}

int module_shutdown(void *module)
{
	delusrcommand(module, "roll");
	delusrcommand(module, "attack");
	return(0) ;
}

static diereturn attroll()
{
	diereturn dice;
	dice.d1 = 1 + (rand() % 20);
	dice.d2 = 1 + (rand() % 20);
	dice.result = 0;

	if (dice.d1 == 20 && dice.d2 == 20) {
		dice.result = dice.result + 5;
		return (dice);
	}

	if (dice.d1 == 20) {
		dice.result = dice.result + 2;
	} else if (dice.d1 >= 15) {
		dice.result++;
	}

	if (dice.d2 == 20) {
		dice.result = dice.result + 2;
	} else if (dice.d2 >= 15) {
		dice.result++;
	}


	return (dice);
}

int dicemod_roll(const char *replyto, struct userlist *user, const char * UNUSED(data))
{
	diereturn dice;
	dice = attroll();
	
	msgreply(user, replyto, "%s rolls a %i and a %i and does %i damage\n",
		unick(user), dice.d1, dice.d2, dice.result);
	return(0) ;
	
}

int dicemod_attack(const char *replyto, struct userlist *user, char *data)
{
	char *monster;
	char output[500], tmpout[30];
	int cnt, i;
	
	diereturn dice[11];

	if (data == NULL) {
		msgreply(user, replyto,
			"useage ATTACK number name attacks victim\n");
		return(0) ;
	}

	data = strtok(data, " ");
	cnt = atoi(data);

	data = strtok(NULL, " ");
	monster = data;

	data = strtok(NULL, " ");
	data = strtok(NULL, "\n");

	if ((cnt < 1) || (cnt > 10)) {
		msgreply(user, replyto,
			"Please use a number from 1 to 10\n");
		return(0) ;
	}

	for (i = 1; i <= cnt; i++) {
		dice[i] = attroll();
	}

	msgreply(user, replyto, "%i %s attack %s!!\n", cnt, monster, data);

	sprintf(output, "          (  %i  ", 1);

	for (i = 2; i <= cnt; i++) {
		sprintf(tmpout, "| %2i  ", i);
		strcat(output, tmpout);
	}

	strcat(output, ") \n");

	msgreply(user, replyto, "%s", output);

	sprintf(output, "rolling.. (%2i,%2i", dice[1].d1, dice[1].d2);

	for (i = 2; i <= cnt; i++) {
		sprintf(tmpout, "|%2i,%2i", dice[i].d1, dice[i].d2);
		strcat(output, tmpout);
	}

	strcat(output, ") \n");

	msgreply(user, replyto, "%s", output);

	sprintf(output, "damage:   (  %i  ", dice[1].result);

	for (i = 2; i <= cnt; i++) {
		sprintf(tmpout, "|  %i  ", dice[i].result);
		strcat(output, tmpout);
	}

	strcat(output, ") \n");

	msgreply(user, replyto, "%s", output);

	return (0);
}
