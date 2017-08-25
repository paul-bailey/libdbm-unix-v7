#ifndef DBM_PRIV_H
#define DBM_PRIV_H

#include "egdbm.h"

#ifndef EXPORT
# ifdef __GNUC__
#  define EXPORT __attribute__((visibility("default")))
# else
#  error "None of this works if ain't gnu"
#  define EXPORT
# endif
#endif

struct Database {
       long bitno;
       long maxbno;
       long blkno;
       long hmask;

       long pblksiz;
       long dblksiz;
       int pblkshift;
       int dblkshift;

       /* name of .pag and .dir files */
       char pagbuf[PBLKSIZ] __attribute__((aligned(2)));
       char dirbuf[DBLKSIZ] __attribute__((aligned(2)));

       /* descriptors of .pag and .dir files */
       int dirfd;
       int pagfd;
       int access_oldb;
       int getbit_oldb;
};

/* datum.c */
extern int cmpdatum(datum d1, datum d2);

/* bits.c */
extern void setbit(Database *db);
extern int getbit(Database *db);

/* forder.c */
extern void dbm_access(Database *db, long hash);

/* buffer.c */
extern datum makdatum(char buf[PBLKSIZ], int idx);
extern void delitem(char buf[PBLKSIZ], int idx);
extern int additem(char buf[PBLKSIZ], datum item);

/*
 * As a general rule, shared-object libraries shouldn't print to
 * standard error.
 */
#if (DBG_MESSAGING == 1)
# include <stdio.h>
# define DBG(msg, args...) fprintf(stderr, msg, ## args)
#else
# define DBG(msg, args...) do { (void)0; } while (0)
#endif

#endif /* DBM_PRIV_H */
