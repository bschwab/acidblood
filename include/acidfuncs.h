extern int match(const char *,const char *) ;
extern void safefree(char *) ;

extern void strip_char_from_end(char,char *) ;
extern int streq(const char *, const char *) ;
extern int strcaseeq(const char *, const char *) ;

extern void irc_sprintf(struct socket_info *, const char *,...)  __attribute__ ((format(printf, 2, 3))) ;
extern void send_notice(struct socket_info *, const char *, const char *, ...)  __attribute__ ((format(printf, 3, 4)));
extern void send_action(struct socket_info *, const char *, const char *, ...)  __attribute__ ((format(printf, 3, 4)));
extern void send_ctcp(struct socket_info *, const char *, const char *, ...) __attribute__ ((format(printf, 3, 4)));
extern void ctcp_reply(struct userlist *user, const char *format, ...) __attribute__ ((format(printf, 2, 3))) ;
extern void privmsg(struct socket_info *, const char *, const char *, ...)  __attribute__ ((format(printf, 3, 4)));
extern void msguser(struct userlist *user, const char *format, ...) __attribute__ ((format(printf, 2, 3))); 
extern void msgreply(struct userlist *user, const char *replyto, const char *,...)  __attribute__ ((format(printf, 3, 4)));
extern void delnetworkmodule(void *module) ;

extern const char *GetNick(struct socket_info *) ;
extern char GetCommandChar(void) ;
extern void send_packet(struct socket_info *, const char *) ;

extern int check_level(struct userlist *);
extern int check_pass(const char *userpass, const char *serverpass);
extern void SetAccess(struct userlist *, const char *) ;
extern void SetChanAccess(struct userlist *,const char *, const char *) ;
extern int GetChannelAccess(struct userlist *, const char *) ;
extern int GetAccess(struct userlist *) ;
extern int access_too_low(struct userlist *, const char *) ;

extern char *unick(struct userlist *) ;
extern struct socket_info *usinfo(struct userlist *) ;

struct commandlist *addcommand(void *, struct commandlist *, const char *, int (*function)(), int, int) ;
struct commandlist *delcommand(void *, struct commandlist *, const char *) ;
struct commandlist *delcommandmodule(void *, struct commandlist *) ;

extern void addservercommand(void *, const char *, int (*function) (socket_info *, struct inputstruct *)) ;
extern void delservercommand(void *, const char *) ;
extern void delservercommandmodule(void *) ;
 
extern void addusrcommand(void *, const char *, int (*function)(const char *, struct userlist *, const char *), int) ;
extern void addrwusrcommand(void *, const char *, int (*function)(const char *, struct userlist *, char *), int) ;
extern void delusrcommand(void *, const char *) ;
extern void delusrcommandmodule(void *) ;

extern void addctcp(void *, const char *, int (*function) (struct userlist *, const char *)) ;
extern void delctcp(void *, const char *) ;
extern void delctcpmodule(void *) ;

extern void addnumeric(void *, int ,void (*function)(socket_info *, char *)) ;
extern void delnumeric(void *, int) ;
extern void delnumericmodule(void *) ;

extern void addtimer(void *, time_t, void (*function)()) ;
extern void schedulein(void *, int, void (*function)(void)) ;
extern void deltimer(void *, time_t) ;
extern void deltimermodule(void *) ;
extern void CheckTimers(void) ;

extern void addtrigger(void *, int , void (*function)(int, struct socket_info *, char *, char *, char *)) ;
extern void deltrigger(void *, int) ;
extern void deltriggermodule(void *) ;

extern void botevent(const int, struct socket_info *, const char *, const char *, const char *) ;

/* log functions */
extern void init_log(void) ;
extern void info_log(const char *, ...)  __attribute__ ((format(printf, 1, 2)));
extern void debug_log(const char *, ...)  __attribute__ ((format(printf, 1, 2)));
extern void close_log(void) ;
extern void display_error(const char *, ...)  __attribute__ ((format(printf, 1, 2)));

extern int IsInChannel(char *, char *,struct server_list *);
extern struct userlist *GetFromChannel(const char *, const char *, struct server_list *);

extern void parse_channels(char *) ;

/* config functions */
extern struct cnfs *addconf(struct cnfs *, const char *, void (*)(char *)) ;
extern struct cnfs *clearconf(struct cnfs *) ;
extern void read_config(struct cnfs *, const char *) ;
