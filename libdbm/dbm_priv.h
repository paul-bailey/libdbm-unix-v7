#ifndef DBM_PRIV_H
#define DBM_PRIV_H

#include "egdbm.h"
#include <stddef.h>

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
       char *pagbuf;
       char *dirbuf;

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

/* dbm_access.c */
extern void dbm_access(Database *db, long hash);

/* forder.c */
extern void forder_hash(Database *db, long hash);

/* buffer.c */
extern datum makdatum(char *buf, int idx, size_t pblksiz);
extern void delitem(char *buf, int idx, size_t pblksiz);
extern int additem(char *buf, datum item, size_t pblksiz);

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
