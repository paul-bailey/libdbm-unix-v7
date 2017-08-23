#ifndef DBM_PRIV_H
#define DBM_PRIV_H

#include "dbm.h"

#ifndef EXPORT
# ifdef __GNUC__
#  define EXPORT __attribute__((visibility("default")))
# else
#  define EXPORT
# endif
#endif

struct Database {
       long bitno;
       long maxbno;
       long blkno;
       long hmask;

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
extern datum makdatum(char buf[PBLKSIZ], int n);
extern int cmpdatum(datum d1, datum d2);

/* delete.c */
extern void delitem(char buf[PBLKSIZ], int n);

/* bits.c */
extern void setbit(Database *db);
extern int getbit(Database *db);

/* forder.c */
extern void dbm_access(Database *db, long hash);

/*
 * As a general rule, shared-object libraries shouldn't print to
 * standard error.
 */
#if DBG_MESSAGING
# include <stdio.h>
# define DBG(msg, args...) fprintf(stderr, msg, ## args)
#else
# define DBG(msg, args...) do { (void)0; } while (0)
#endif

#endif /* DBM_PRIV_H */
