#include "tcpd.h"
#include "socket.h"
#include "byte.h"

extern int ndelay_on(int);
extern void uint_pack_big(char[], unsigned int);
extern void uint_unpack_big(char[], unsigned int *);

int socket_tcp(void) {
 int s;

 s = socket(AF_INET, SOCK_STREAM, 0);
 if (s == -1) return -1;
 if (ndelay_on(s) == -1) { close(s); return -1; }
 return s;
}

int socket_bind4(int s, char ip[4], unsigned int port) {
 struct sockaddr_in sa;
 byte_zero(&sa, sizeof sa);
 sa.sin_family = AF_INET;
 uint_pack_big((char *) &sa.sin_port,port);
 byte_copy((char *) &sa.sin_addr,4,ip);
 return bind(s,(struct sockaddr *) &sa,sizeof sa);
}

int socket_bind4_reuse(int s, char ip[4], unsigned int port) {
 int opt = 1;
 setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
 return socket_bind4(s, ip, port);
}

int socket_local4(int s, char ip[4], unsigned int *port) {
 struct sockaddr_in sa;
 int dummy = sizeof sa;

 if (getsockname(s, (struct sockaddr *) &sa, (socklen_t *) &dummy) == -1) return -1;
 byte_copy(ip, 4, (char *) &sa.sin_addr);
 uint_unpack_big((char *) &sa.sin_port, port);
 return 0;
}

int socket_listen(int s, int backlog) {
 return listen(s, backlog);
}

int socket_accept4(int s, char ip[4], unsigned int *port) {
 struct sockaddr_in sa;
 int dummy = sizeof sa;
 int fd;

 fd = accept(s,(struct sockaddr *) &sa, (socklen_t *) &dummy);
 if (fd == -1) return -1;

 byte_copy(ip,4,(char *) &sa.sin_addr);
 uint_unpack_big((char *) &sa.sin_port,port);
 return fd;
}

int socket_tcpnodelay(int s) {
 int opt = 1;
 return setsockopt(s,IPPROTO_TCP,1,&opt,sizeof opt); /* 1 == TCP_NODELAY */
}

int socket_ipoptionskill(int s) {
 return setsockopt(s,IPPROTO_IP,1,(char *) 0, 0); /* 1 == IP_OPTIONS */
}
