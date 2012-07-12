extern void startup(void) ;
extern void acid_shutdown(int) __attribute__((noreturn));
extern void acid_reload(const char *) ;
extern void background(void) ;

extern int module_init(void *) ;
extern int module_shutdown(void *) ;

/* parser functions */
extern int read_user_data(void);
extern int parse_server (struct socket_info *);
extern int get_channel(struct channels *channeldata, char *data, char *key);
extern void SplitInput (char *, struct inputstruct *);
extern void lowerstring(char *) ;

extern void convert_clientinfo(struct socket_info *, char *,struct clientinfo *) ;
extern void build_clientinfo(struct clientinfo *,struct server_list *, char *) ;

/* signal functions */
extern void init_signals(void) ;

/* network functions */
extern void complete_connect(struct socket_info *) ;
extern int connect_to_host(struct socket_info *, char *, int);
extern int register_to_server(struct socket_info *) ;
extern void check_connections(void);
extern void disconnected(struct socket_info *) ;
extern int getline(struct socket_info *) ;
extern struct socket_info *GetNewSocket(void (*handler) (int, socket_info *)) ;
extern struct socket_info *GetModuleSocket(void *module, void (*handler) ()) ;
extern void FreeSocket(struct socket_info *);

/* list functions */
extern int free_list(void);
extern int insert_users(char *, char *, char *, char *, char *);
extern int check_chan(const char *userchan, const char *serverchan);
extern void clear_channels(void) ;
extern int delete_channel(const char *data);
extern int insert_channels(const char *data, const char *key);
extern void join_channels(struct socket_info *);
extern void AddServerConf(char *, int, char *) ;

/* action functions */
extern int handle_ctcp(struct userlist *, char *) ;
extern int handle_server_command(struct socket_info *, struct inputstruct *) ;
extern int handle_numeric(struct socket_info *,struct inputstruct *) ;
extern void CheckConfServers(void) ;
extern void handle_dcc(struct userlist *, char *) ;

/* tracking functions */
extern int EnterChannel(const char *,const char *,struct server_list *) ;
extern int ExitChannel(const char *,struct server_list *) ;
extern void InitTracking(void) ;
extern void AddServer(struct socket_info *, char *,char *) ;
extern void DelServerByName(char *) ;
extern void DelServerBySocket(struct socket_info *) ;
extern struct userlist *AddUserToChannel(struct clientinfo *,char *) ;
extern struct userlist *UpdateUser(struct clientinfo *) ;
extern void DelUserFromChannel(struct clientinfo *,char *) ;
extern void QuitUser(struct clientinfo *) ;
extern void RenameUser(struct clientinfo *,char *) ;
extern void ReapUserList(void) ;
extern void UpdateMsgTime(struct userlist *) ;
extern void UpdateAccess(struct userlist *, const char *) ;
extern struct server_list *GetServerBySocket(struct socket_info *) ;

/* module functions */
extern int loadmodule(const char *) ;
extern int removemodule(const char *) ;
extern void listmods(const char *, struct userlist *) ;

/* module functions */
extern void disconnectonidle(socket_info *) ;
extern void pingonidle(socket_info *) ;
