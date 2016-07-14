#include "tcpd.h"
#include "byte.h"
#include "socket.h"
#include "sig.h"
#include "wait.h"
#include "ptable.h"

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

typedef unsigned short uint16;

unsigned int limit = 50;
unsigned int numchildren = 0;
unsigned int autokill = 0;
int flagkillopts = 1;
int flagdelay = 1;
unsigned int localport, remoteport;

char localip[4];
char remoteip[4];

struct pentry *pt = NULL;

int fd_copy(int to, int from) {
 if (to == from) return 0;
 if (fcntl(from,F_GETFL, 0) == -1) return -1;
 close(to);
 if(fcntl(from,F_DUPFD,to) == -1) return -1;
 return 0;
}

int fd_move(int to,int from) {
  if (to == from) return 0;
  if (fd_copy(to,from) == -1) return -1;
  close(from);
  return 0;
}

void usage(void) {
 printf("\ntcpd: usage: tcpd [ -c limit ] [ -k limit ] host port program\n");
 exit(0);
}

void uint_pack_big(char s[2], unsigned int u) {
 s[1] = u & 255;
 s[0] = u >> 8;
}

void uint_unpack_big(char s[2], unsigned int *u) {
 unsigned int result;
 result = (unsigned char) s[0];
 result <<= 8;
 result += (unsigned char) s[1];
 *u = result;
}

int ndelay_on(int fd) { return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK); }
int ndelay_off(int fd) { return fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0) & ~O_NONBLOCK); }

void eprint(int n, const char *s, ...) {
 const char *types[3] = {"fatal: ", "warn: ", "status: "};
 va_list argptr;
 va_start(argptr, s);
 fprintf(stderr, types[n]);
 vfprintf(stderr, s, argptr);
 fprintf(stderr, "\n");
 va_end(argptr);
}

void printstatus() { eprint(P_STAT, "%d / %d", numchildren, limit); }

void die(int code, char *str) {
 eprint(P_FATAL, str);
 exit(code);
}

void sigchld() {
 int wstat;
 int pid;

 while ((pid = wait_nohang(&wstat)) > 0) {
  fprintf(stderr, "end: pid %d status %d\n", pid, wstat);
  if (autokill != 0) ptable_remove(pt, limit, pid);
  if (numchildren) --numchildren; printstatus();
 }
}

void sigterm() {
 fprintf(stderr, "sig_term caught\n");
 if (autokill != 0) ptable_destroy(pt);
 exit(0);
}

void sigint() {
 fprintf(stderr, "sig_int caught\n");
 if (autokill != 0) ptable_destroy(pt);
 exit(0);
}

int main(int argc,char **argv)
{
 char *hostname, *x;
 int c, s, t;
 unsigned int u;
 unsigned int cpid = 0;

 opterr = 0;

 while ((c = getopt(argc, argv, "dDoO:k:c:")) != -1)
  switch (c) {
	case 'c':
	 limit = atoi(optarg);
	 if (limit == 0) usage();
	 break;
	case 'd': flagdelay = 1; break;
	case 'D': flagdelay = 0; break;
	case 'O': flagkillopts = 1; break;
	case 'o': flagkillopts = 0; break;
	case 'k':
	 autokill = atoi(optarg);
	 if (autokill == 0) usage();
	 break;
	default: abort();
  }
 argc -= optind;
 argv += optind;

 hostname = *argv++;
 if (!hostname) usage();

 x = *argv++;
 if (!x) usage();
 u = 0;
 u = atoi(x);
 if (u != 0) localport = u;
 else usage();

 if (!*argv) usage();

 sig_block(sig_child);
 sig_catch(sig_child,sigchld);
 sig_catch(sig_term,sigterm);
 sig_catch(sig_int,sigint);
 sig_ignore(sig_pipe);

 inet_aton(hostname, (struct in_addr *) &localip);

 if (autokill != 0) pt = ptable_init(limit);

 s = socket_tcp();
 if (s == -1) die(111, "unable to create socket");
 if (socket_bind4_reuse(s,localip,localport) == -1) die(111, "unable to bind");
 if (socket_local4(s,localip,&localport) == -1) die(111, "unable to get local address");
 if (socket_listen(s,20) == -1) die(111, "unable to listen");
 ndelay_off(s);

 fprintf(stderr, "bind: %s:%d\n", hostname, localport);

 close(0);
 close(1);
 printstatus();

 for (;;) {
   while (numchildren >= limit) {
    if (autokill != 0) ptable_autokill(pt, limit, autokill);
    sig_pause();
   }

   sig_unblock(sig_child);
   t = socket_accept4(s,remoteip,&remoteport);
   sig_block(sig_child);

   if (t == -1) continue;
   ++numchildren; printstatus();
   fprintf(stderr, "inbound connection from %d.%d.%d.%d:%d\n", (unsigned char) remoteip[0], (unsigned char) remoteip[1], (unsigned char) remoteip[2], (unsigned char) remoteip[3], remoteport);

   if (autokill != 0) ptable_autokill(pt,limit,autokill);

   cpid = fork();
   switch(cpid) {
	case 0:
	 close(s);
	 if(flagkillopts) socket_ipoptionskill(t);
	 if(!flagdelay) socket_tcpnodelay(t);
	 if((fd_move(0,t) == -1) || (fd_copy(1,0) == -1)) die(111,"unable to setup descriptors");
	 sig_uncatch(sig_child);
	 sig_unblock(sig_child);
	 sig_uncatch(sig_term);
	 sig_uncatch(sig_int);
	 sig_uncatch(sig_pipe);
	 if(execve(*argv,argv,NULL) == 0) {
	   close(t);
	   exit(0);
	 } else {
	   die(111, "unable to run argv");
	 }
	 break;
	case -1:
	 // unable to fork
	 eprint(P_WARN,"unable to fork");
	 --numchildren; printstatus();
	 break;
	default:
	 fprintf(stderr, "fork: child pid %d\n", cpid);
	 if (autokill != 0) ptable_set(pt, limit, cpid, time(NULL));
	 break;
   }
   close(t);
 }
}
