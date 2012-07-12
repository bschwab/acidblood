//struct socket_info socket_pool[200] ;
#define NET_READ 	0
#define NET_WRITE 	1
#define NET_ERROR 	-1 

#define net_connected 		0x0001
#define net_no_retry  		0x0002
#define net_connecting  	0x0004
#define net_writeable 		0x0008
#define net_readable 		0x0010
#define net_irc_protocol	0x0020
#define net_registered		0x0040
#define net_unconnectable	0x0080

#define ClearFlags(x) 		((x)->flags = 0) 

#define IsConnected(x) 		((x)->flags & net_connected)
#define IsConnecting(x) 	((x)->flags & net_connecting)
#define IsReadable(x) 		((x)->flags & net_readable)
#define IsWriteable(x) 		((x)->flags & net_writeable)
#define IsIrcServer(x)		((x)->flags & net_irc_server) 
#define IsRegistered(x) 	((x)->flags & net_registered)
#define IsNotConnectable(x) 	((x)->flags & net_unconnectable)

#define SetConnected(x)		((x)->flags |=  net_connected)
#define NotConnected(x) 	((x)->flags &= ~net_connected)
#define SetConnecting(x) 	((x)->flags |=  net_connecting)
#define NotConnecting(x) 	((x)->flags &= ~net_connecting)
#define SetReadable(x) 		((x)->flags |=  net_readable)
#define NotReadable(x) 		((x)->flags &= ~net_readable)
#define SetWriteable(x)		((x)->flags |=  net_writeable)
#define NotWriteable(x)		((x)->flags &= ~net_writeable)
#define SetIrcProtocol(x) 	((x)->flags |=  net_irc_protocol) 
#define NotIrcProtocol(x) 	((x)->flags &= ~net_irc_protocol)
#define SetRegistered(x) 	((x)->flags |=  net_registered)
#define NotRegistered(x)	((x)->flags &= ~net_registered)
#define NotConnectable(x)	((x)->flags &= ~net_unconnectable) 
#define SetConnectable(x)	((x)->flags |=  net_unconnectable)
#define SetPongRecieved(x) 	((x)->flags |=  net_pongrecv)
