#ifndef PTABLE_H
#define PTABLE_H

struct pentry {
 unsigned int pid;
 unsigned int start;
};

extern struct pentry *ptable_init(int);
extern int ptable_proc(struct pentry *, int, int, int, int);
#define ptable_set(p,n,x,s) (ptable_proc((p),1,(n),(x),(s)))
#define ptable_remove(p,n,x) (ptable_proc((p),2,(n),(x),0))
#define ptable_gettime(p,n,x) (ptable_proc((p),3,(n),(x),0))
#define ptable_autokill(p,n,s) (ptable_proc((p),4,(n),0,(s)))
extern void ptable_destroy(struct pentry *);

#endif
