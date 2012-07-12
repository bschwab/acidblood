/* Acidblood network routines */
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

/* should define this in the makefile */
// #define IPv6


#include "acidblood.h"
#include "acidfuncs.h"
#include "internal.h"
#include "network.h"
#include "extern.h"
#include "startup.h"
#include "list.h"

#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>


/*
 *  Prevent a compile error on OS that don't support MSG_NOSIGNAL.
 *  We trap SIGPIPE anyways so this shouldn't cause a crash.  
 */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

static void send_error(int err)
{
	switch (err) {
	case EBADF:{
			info_log("send error: EBADF (bad file descriptor)\n");
			return;
		}
	case EAGAIN:{
			info_log("send error: EAGAIN(would have blocked)\n");
			return;
		}
	case ECONNRESET:{
			info_log("send error: ECONNRESET(connection reset by peer)\n");
			return;
		}
	case EFAULT:{
			info_log("send error: EFAULT(invalid userspace address)\n");
			return;
		}
	case EINTR:{
			info_log("send error: EINTR(Interrupted by signal)\n");
			return;
		}
	case EINVAL:{
			info_log("send error: EINVAL(Invalid arguement)\n");
			return;
		}
	case EMSGSIZE:{
			info_log("send error: EMSGSIZE(message size too big)\n");
			return;
		}
	case ENOTSOCK:{
			info_log("send error: ENOTSOCK:(not a socket)\n");
			return;
		}
	case ENOTCONN:{
			info_log("send error: ENOTCONN:(socket not connected\n");
			return;
		}
		case EOPNOTSUPP:{
			info_log("send error: EOPNOTSUPP\n");
			return;
		}
	case EPIPE:{
			info_log("send error: EPIPE\n");
			return;
		}
	default:{
			info_log("Send: (%i)unknown errror\n",err);
			return;
		}
	}

}
void send_packet(struct socket_info *sinfo, const char *buff)
{
	/* if MSG_NOSIGNAL is not passed and the remote side drops
	 * the connection mid send the prgram gets a SIGPIPE
	 * this has the lovely side affect of shutting the program off
	 * if your not trapping SIGPIPE.  I don't want to know what sort of
	 * drugs it took to make someone think that was a bright idea.
	 * addendum: MSG_NOSIGNAL only works on linux trapping SIGPIPE 
	 * seems to be the way to go everywhere else
	 */

	if ((send(sinfo->fd, buff, strlen(buff), MSG_NOSIGNAL)) == -1) {

		send_error(errno) ;
                disconnected(sinfo);
	}
	sinfo->lastsndpacket=currtime ;
#ifdef DEBUGLINE
		debug_log("line< %s", buff);
#endif
}

void init_networking()
{
	int cnt;

	for (cnt = 0; cnt < FD_SETSIZE && cnt < 200; cnt++) {
		socket_pool[cnt].flags = 0;
		socket_pool[cnt].buffpos = 0;
		socket_pool[cnt].handler = NULL;
	}
}

static void network_error(int result, int err)
{
	if (result == 0) {
		/* timeout */
		return;
	}
	if (result == -1) {
		/* error */

		if (err == EINTR) {
			/* we were interrupted */
			return;
		}
		if (err == EBADF) {
			info_log("network: bad file descriptor passed to select\n");
			return;
		}
		if (err == ENOMEM) {
			info_log("network: out of memory\n");
			return;
		}
		if (err == EINVAL) {
			info_log("network: invalid file descriptor passed to select\n");
			return;
		}

		info_log("network: select unknown errorno %i\n", err);
		return;
	}
	info_log("network: select unknown error %i\n", result);
}

static void keepalive(socket_info *sinfo) 
{
	const char keepstr[]="\n" ;
	send_packet(sinfo,keepstr) ;
}

void check_connections()
{
	int len;
	struct timeval tv;
	int cnt = 0;
	fd_set infd, outfd, errorfd;

	tv.tv_sec = 5 ;
	tv.tv_usec = 0;

	FD_ZERO(&infd);
	FD_ZERO(&outfd);
	FD_ZERO(&errorfd);

	for (cnt = 0; cnt < FD_SETSIZE && cnt < 200; cnt++) {
		if (socket_pool[cnt].handler != NULL) {
			if (IsReadable(&socket_pool[cnt])) {
				FD_SET(socket_pool[cnt].fd, &infd);
				FD_SET(socket_pool[cnt].fd, &errorfd);
				socket_pool[cnt].lastrcvpacket=currtime ;
			}

			if (IsWriteable(&socket_pool[cnt])) {
				FD_SET(socket_pool[cnt].fd, &outfd);
				FD_SET(socket_pool[cnt].fd, &errorfd);
			}
		}
	}
	len = select(FD_SETSIZE, &infd, &outfd, &errorfd, &tv);

	if (len < 0) {
		network_error(len, errno);
		return;
	}
	
	for (cnt = 0; cnt < FD_SETSIZE && cnt < 200; cnt++) {
		if (FD_ISSET(socket_pool[cnt].fd, &errorfd)) {
			debug_log("select error\n") ;
			disconnected(&socket_pool[cnt]);
		}

		if (FD_ISSET(socket_pool[cnt].fd, &infd)) {
			if(socket_pool[cnt].handler!=NULL) {
				socket_pool[cnt].handler(NET_READ,
							 &socket_pool[cnt]);
			}
		}

		if (FD_ISSET(socket_pool[cnt].fd, &outfd)) {
			SetReadable(&socket_pool[cnt]);
			if(socket_pool[cnt].handler!=NULL) {
				socket_pool[cnt].handler(NET_WRITE,
							 &socket_pool[cnt]);
			}
		}
		if(socket_pool[cnt].idle != NULL) {
			if(socket_pool[cnt].timeout+socket_pool[cnt].lastrcvpacket < currtime)
				socket_pool[cnt].idle(&socket_pool[cnt]) ;
		}
		if(socket_pool[cnt].keepalive > 0 ) { 
			if(socket_pool[cnt].keepalive+socket_pool[cnt].lastsndpacket < currtime ){
				keepalive(&socket_pool[cnt]) ;
			}
		}
	}
}


void delnetworkmodule(void *module)
{
	int cnt ;

	for (cnt = 0; cnt < FD_SETSIZE && cnt < 200; cnt++) {
		if (socket_pool[cnt].module == module) {
			FreeSocket(&socket_pool[cnt]);
		}
	}
}

int getline(struct socket_info *sinfo)
{
	while (1) {

		if (recv(sinfo->fd, &sinfo->buff[sinfo->buffpos], 1, 0) < 0) {
			return (0);
		}

		totalbytes++;
		
		/* all strings end in \r\n, so look for the \n */
		if (sinfo->buff[sinfo->buffpos] == '\n') {
			sinfo->buff[sinfo->buffpos] = '\0';
			if(sinfo->buff[sinfo->buffpos-1] == '\r')
				sinfo->buff[sinfo->buffpos-1] = '\0';
			sinfo->lastrcvpacket=currtime ;
			#if 0
			debug_log("network: found delimiter, done\n");
			#endif
			break;
		}

    		/* buffer size is 1500 */
                if (sinfo->buffpos > 1500) {
			/* hmm, looks like buffer got away somewhere, 
			   return an error 
			 */
			debug_log("network: buffer too large: disconnecting\n");
			sinfo->buffpos = 0;
			disconnected(sinfo) ;
		}
		sinfo->buffpos++;
	}

#ifdef DEBUGLINE
	debug_log("line> %s\n", sinfo->buff);
#endif
	sinfo->buffpos = 0;
	return (1);
}

static void connect_error(int err)
{
	switch (err) {
	case EBADF:{
			info_log("connect error: bad file descriptor \n");
			return;
		}
	case EADDRINUSE:{
			info_log("connect error: address in use \n");
			return;
		}
	case EALREADY:{
			info_log("connect error: connect already in progress \n");
			return;
		}
	case EFAULT:{
			info_log("connect error: address fault\n");
			return;
		}
	case ENOTSOCK:{
			info_log("connect error: not a socket\n");
			return;
		}
	case EISCONN:{
			info_log("connect error: socket in use\n");
			return;
		}
	case ECONNREFUSED:{
			info_log("connect error: connection refused\n");
			return;
		}
	case ETIMEDOUT:{
			info_log("connect error: connection timed out\n");
			return;
		}
	case ENETUNREACH:{
			info_log("connect error: network unreachable\n");
			return;
		}
	default:{
			info_log("unknown errror(%i)\n",err);
			return;
		}



	}
}

int connect_to_host(struct socket_info *sinfo, char *server, int port)
{
	struct sockaddr_in sockin;
	struct hostent *hp;
	int res;
#ifdef IPv6
	struct sockaddr_in6 sa6;
#endif

	NotConnected(sinfo);
	SetConnecting(sinfo);

	sockin.sin_family = AF_INET;
	sockin.sin_port = htons(port);
#ifdef IPv6
	if ((hp = gethostbyname2(server, AF_INET6)) != NULL) {
		info_log("Will use IPv6\n");
		sa6.sin6_family = AF_INET6;
	}
	if (!hp)
#endif
		hp = gethostbyname(server);

	if (!hp) {
		info_log("Connect Error: Unknown host %s\n", server);
		disconnected(sinfo) ;
		return (-1);
	}
#ifdef IPv6
	if (sa6.sin6_family == AF_INET6) {
		if ((sinfo->fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
			info_log("Connect Error: Cannot create IPv6 socket!\n");
			disconnected(sinfo) ;
			return (-1);
		}
		memcpy(&sa6.sin6_addr, hp->h_addr, hp->h_length);
	} else {
#endif
		if ((sinfo->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			info_log("Connect Error: Cannot create socket!\n");
			disconnected(sinfo) ;
			return (-1);
		}

		bcopy(hp->h_addr, (struct in_addr *) &sockin.sin_addr,
		      hp->h_length);
#ifdef IPv6
	}
#endif
	fcntl(sinfo->fd, F_SETFL, O_NONBLOCK);
	sinfo->lastsndpacket=currtime ;
	sinfo->lastrcvpacket=currtime ;
#ifdef IPv6
	if (sa6.sin6_family == AF_INET6) {
		sa6.sin6_port = htons(port);
		// The void cast is there to shut GCC up
		res =
		    connect(sinfo->fd, (struct sockaddr *)(void *) &sa6,
			    sizeof(sa6));
	} else {
#endif
		// The void cast is there to shut GCC up
		res =
		    connect(sinfo->fd, (struct sockaddr *)(void *) &sockin,
			    sizeof(sockin));
#ifdef IPv6
	}
#endif
	if (res < 0) {
		switch (errno) {
		case EINPROGRESS:{
				SetWriteable(sinfo);
				return (0);
				break;
			}
		case EINTR:{
				SetWriteable(sinfo);
				return (0);
				break;
			}
		default:{
				connect_error(errno);
				close(sinfo->fd);
				disconnected(sinfo)  ;
				return (-1);
				break;
			}
		}
	}
	SetReadable(sinfo);
	sinfo->handler(NET_WRITE, sinfo);
	return (0);
}


void disconnected(struct socket_info *sinfo)
{
	NotConnected(sinfo);
	NotConnecting(sinfo);
	NotWriteable(sinfo);
	NotReadable(sinfo);
	sinfo->pingtimeout=0 ;
	sinfo->handler(NET_ERROR, sinfo);
	                                                                         
}

struct socket_info *GetNewSocket(void (*handler) ())
{
	int cnt;

	for (cnt = 0; cnt < FD_SETSIZE && cnt < 200; cnt++) {
		if (socket_pool[cnt].handler == NULL) {
			ClearFlags(&socket_pool[cnt]);
			socket_pool[cnt].lastrcvpacket=0 ;
			socket_pool[cnt].lastsndpacket=0 ;
			socket_pool[cnt].pingtimeout=0 ;
			socket_pool[cnt].buffpos = 0;
			socket_pool[cnt].handler = handler;
			socket_pool[cnt].module = NULL ;
			socket_pool[cnt].idle = NULL ;
			socket_pool[cnt].timeout = 0 ;
			return (&socket_pool[cnt]);
		}
	}
	info_log("GetNewSocket: No Available file descriptors\n");
	return (NULL);
}

struct socket_info *GetModuleSocket(void *module, void (*handler) ())
{
	struct socket_info *sinfo ;
	
	sinfo = GetNewSocket(handler) ;
	
	if(sinfo == NULL) {
		return(NULL) ;
	}
	
	sinfo->module = module ;
	
	return(sinfo) ;
}

void FreeSocket(struct socket_info *sinfo)
{
	close(sinfo->fd);
	ClearFlags(sinfo);
	sinfo->buffpos = 0;
	sinfo->handler = NULL ;
	sinfo->module = NULL ;
	sinfo->idle = NULL ;
	bzero(sinfo->buff, sizeof(sinfo->buff));
}

static void disconnect(socket_info *sinfo, const char *reason)
{
	irc_sprintf(sinfo, "QUIT :%s\n", reason) ;
	info_log("QUIT :%s\n", reason) ;
        disconnected(sinfo) ;
}

void disconnectonidle(socket_info *sinfo)
{
	disconnect(sinfo, "connection timed out") ;
}

void pingonidle(socket_info *sinfo)
{
	if(sinfo->pingtimeout > 0) {
		if(sinfo->pingtimeout < currtime) {
			disconnect(sinfo, "ping timeout") ;
		}
		return ;
	}
                                                                        
	sinfo->pingtimeout=currtime+sinfo->timeout ;
	irc_sprintf(sinfo, "PING :%i\n", (int)currtime) ;
}
