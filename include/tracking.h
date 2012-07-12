
struct server_list {
	struct server_list *prev ;
	char *nick ;
	char *name ;
	struct socket_info *sinfo ; 
	struct channellist *clist ;
	struct userlist *ulist ;
	struct server_list *next ;
} ;
  
struct achannel {
	char *name ;
	char *topic ;
	struct userlist *users ;
} ;

struct channellist {
	struct channellist *prev ;
	struct achannel *channel ;
	struct channellist *next ;
} ;

struct auser {
	char *nick ;
	char *ident ;
	char *address ;
	time_t lastmsg ;
	struct server_list *server ;
	struct channellist *channels ;
} ; 

void FreeUser(struct auser *) ;
void FreeChannel(struct achannel *) ;
void FreeServer(struct server_list *) ;
void LinkUserToChannel(struct auser *, char *) ;
void DelUser(struct userlist *) ;
void DelServer(struct server_list *) ;
struct channellist *GetChannel(const char *, struct server_list *) ;
struct channellist *AddChannel(struct auser *, const char *) ;

