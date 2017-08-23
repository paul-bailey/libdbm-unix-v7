#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void
hmask_cycle(Database *db, long hash)
{
        long hmask;
        for (hmask = 0;; hmask = (hmask << 1) + 1) {
                db->blkno = hash & hmask;
                db->bitno = db->blkno + hmask;
                if (getbit(db) == 0)
                        break;
        }
        db->hmask = hmask;
}

static void
chkblk(char buf[PBLKSIZ])
{
        short *sp;
        int t, i;

        sp = (short *)buf;
        t = PBLKSIZ;
        for (i = 0; i < sp[0]; i++) {
                if (sp[i + 1] > t)
                        goto bad;
                t = sp[i + 1];
        }
        if (t < (sp[0] + 1) * sizeof(short))
                goto bad;
        return;

bad:
        fprintf(stderr, "bad block\n");
        abort();
        memset(buf, 0, PBLKSIZ);
}

void
dbm_access(Database *db, long hash)
{
        hmask_cycle(db, hash);

        if (db->blkno != db->access_oldb) {
                memset(db->pagbuf, 0, PBLKSIZ);
                lseek(db->pagfd, db->blkno * PBLKSIZ, 0);
                read(db->pagfd, db->pagbuf, PBLKSIZ);
                chkblk(db->pagbuf);
                db->access_oldb = db->blkno;
        }
}

static long
hashinc(Database *db, long hash)
{
        long bit;

        hash &= db->hmask;
        bit = db->hmask + 1;
        for (;;) {
                bit >>= 1;
                if (bit == 0)
                        return 0L;
                if ((hash & bit) == 0)
                        return hash | bit;
                hash &= ~bit;
        }
}

static datum
firsthash(Database *db, long hash)
{
        int i;
        datum item, bitem;

        for (;;) {
                dbm_access(db, hash);
                bitem = makdatum(db->pagbuf, 0);
                for (i = 2;; i += 2) {
                        item = makdatum(db->pagbuf, i);
                        if (item.dptr == NULL)
                                break;
                        if (cmpdatum(bitem, item) < 0)
                                bitem = item;
                }
                if (bitem.dptr != NULL)
                        return bitem;
                hash = hashinc(db, hash);
                if (hash == 0)
                        return item;
        }
}

Database *
dbminit(char *file)
{
        struct stat statb;
        Database *db;

        db = malloc(sizeof(*db));
        if (!db)
                goto emalloc;

        strcpy(db->pagbuf, file);
        strcat(db->pagbuf, ".pag");
        db->pagfd = open(db->pagbuf, 2);
        if (db->pagfd < 0)
                goto epagfd;

        strcpy(db->pagbuf, file);
        strcat(db->pagbuf, ".dir");
        db->dirfd = open(db->pagbuf, 2);
        if (db->dirfd < 0)
                goto edirfd;

        fstat(db->dirfd, &statb);
        db->maxbno = statb.st_size * BYTESIZ - 1;
        db->access_oldb = -1;
        db->getbit_oldb = -1;
        return db;

        /* close db->dirfd if more err proc. before this */
edirfd:
        close(db->pagfd);
epagfd:
        free(db);
emalloc:
        fprintf(stderr, "cannot init DBM\n");
        return NULL;
}

long
forder(Database *db, datum key)
{
        hmask_cycle(db, calchash(key));
        return db->blkno;
}

datum
fetch(Database *db, datum key)
{
        int i;
        datum item;

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        return item;
                if (cmpdatum(key, item) == 0) {
                        item = makdatum(db->pagbuf, i + 1);
                        if (item.dptr == NULL)
                                fprintf(stderr, "items not in pairs\n");
                        return item;
                }
        }
}

datum
firstkey(Database *db)
{
        return firsthash(db, 0L);
}

datum
nextkey(Database *db, datum key)
{
        int i;
        datum item, bitem;
        long hash;
        int f;

        hash = calchash(key);
        dbm_access(db, hash);
        f = 1;
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (cmpdatum(key, item) <= 0)
                        continue;
                if (f || cmpdatum(bitem, item) < 0) {
                        bitem = item;
                        f = 0;
                }
        }
        if (f == 0)
                return bitem;
        hash = hashinc(db, hash);
        if (hash == 0)
                return item;
        return firsthash(db, hash);
}
