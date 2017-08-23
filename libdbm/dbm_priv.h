#ifndef DBM_PRIV_H
#define DBM_PRIV_H

#include "dbm.h"

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

/* dbm.c */
extern void dbm_access(Database *db, long hash);

#endif /* DBM_PRIV_H */
