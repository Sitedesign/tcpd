#include "tcpd.h"
#include "wait.h"

int wait_nohang(wstat) int *wstat;
{
  return waitpid(-1,wstat,WNOHANG);
}
