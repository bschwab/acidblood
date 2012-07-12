/* structure for bot information from config file */
struct botstruct {
	char *user;
	char *nick;
	char *altnick;
	char *fname;
	char *channel;
	char *server;
	char *srvpass;
	char *awaymsg;
	char *mode;
	char commandchar;
	int ctcp;
	int autoop;
	int reqpass;
	int port;
	char *ver;
	int ns;
	char *nspass;
	char *image;
	int keepalive;
	int timeout;
	int maxtries ;
	int cmddelay ;
	int ctcpdelay ;
	int oplevel ;
	int hoplevel ;
	int voicelevel ;
	int floodexempt ;
	time_t starttime ;
};

/* link list of user entries */
struct userdata {
	struct userdata *prev;
	char *usernick;
	char *userip;
	int userstatus;
	char *userchan;
	char *userpass;
	struct userdata *next;
};

/* channel list */
struct channels {
	struct channels *prev;
	char *name;
	char *key;
	struct channels *next;
};
