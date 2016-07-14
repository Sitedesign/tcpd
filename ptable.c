#include "tcpd.h"
#include "ptable.h"

struct pentry * ptable_init(int num) {
 struct pentry * pt;
 pt = malloc(sizeof(struct pentry) * num);
 if ( pt == NULL ) return pt;
 memset(pt, 0, sizeof(struct pentry) * num);
 return pt;
}

int ptable_proc(struct pentry *pt, int task, int num1, int pid, int num2) {
 int i, s, now;
 now = time(NULL);
 for (i=0; i<num1; i++) {
  switch (task) {
   case 1: // set (num2 - starttime)
    if (pt[i].pid == 0) {
	pt[i].pid = pid;
	pt[i].start = num2;
	return 0;
    }
    break;
   case 2: // remove
    if (pt[i].pid == pid) {
	pt[i].pid = 0;
	pt[i].start = 0;
	return 0;
    }
    break;
   case 3: // gettime
    if (pt[i].pid == pid) return pt[i].start;
    break;
   case 4: // autokill (num2 - lifetime)
    if (pt[i].pid == 0) continue;
    if ((pt[i].start + num2) < now) {
     s = kill(pt[i].pid, 9);
     fprintf(stderr, "autokill: pid %d status %d\n", pt[i].pid, s);
     pt[i].pid = 0;
     pt[i].start = 0;
    }
    break;
  }
 }
 return -1;
}

void ptable_destroy(struct pentry * pt) {
 free(pt);
}
