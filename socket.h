#ifndef SOCKET_H
#define SOCKET_H

extern int socket_tcp(void);

extern int socket_bind4(int,char *,unsigned int);
extern int socket_bind4_reuse(int,char *,unsigned int);
extern int socket_listen(int,int);
extern int socket_accept4(int,char *,unsigned int *);
extern int socket_local4(int,char *,unsigned int *);
extern int socket_tcpnodelay(int);
extern int socket_ipoptionskill(int);

#endif
