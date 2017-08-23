#include "dbm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Database {
       long bitno;
       long maxbno;
       long blkno;
       long hmask;

       char pagbuf[PBLKSIZ] __attribute__((aligned(2)));
       char dirbuf[DBLKSIZ] __attribute__((aligned(2)));

       int dirf;
       int pagf;
       int access_oldb;
       int getbit_oldb;
};

static void
clearbuf(char *cp, int n)
{
        memset(cp, 0, n);
}

static int
getbit(Database *db)
{
        long bn;
        int b, i, n;

        if (db->bitno > db->maxbno)
                return 0;

        n = db->bitno % BYTESIZ;
        bn = db->bitno / BYTESIZ;
        i = bn % DBLKSIZ;
        b = bn / DBLKSIZ;
        if (b != db->getbit_oldb) {
                clearbuf(db->dirbuf, DBLKSIZ);
                lseek(db->dirf, (long)b * DBLKSIZ, 0);
                read(db->dirf, db->dirbuf, DBLKSIZ);
                db->getbit_oldb = b;
        }

        if (db->dirbuf[i] & (1 << n))
                return 1;
        return 0;
}

static void
setbit(Database *db)
{
        long bn;
        int i, n, b;

        if (db->bitno > db->maxbno) {
                db->maxbno = db->bitno;
                getbit(db);
        }
        n = db->bitno % BYTESIZ;
        bn = db->bitno / BYTESIZ;
        i = bn % DBLKSIZ;
        b = bn / DBLKSIZ;
        db->dirbuf[i] |= 1<<n;
        lseek(db->dirf, (long)b * DBLKSIZ, 0);
        write(db->dirf, db->dirbuf, DBLKSIZ);
}

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
        printf("bad block\n");
        abort();
        clearbuf(buf, PBLKSIZ);
}

static void
dbm_access(Database *db, long hash)
{
        hmask_cycle(db, hash);

        if (db->blkno != db->access_oldb) {
                clearbuf(db->pagbuf, PBLKSIZ);
                lseek(db->pagf, db->blkno * PBLKSIZ, 0);
                read(db->pagf, db->pagbuf, PBLKSIZ);
                chkblk(db->pagbuf);
                db->access_oldb = db->blkno;
        }
}

static int
cmpdatum(datum d1, datum d2)
{
        int n;

        n = d1.dsize;
        if (n != d2.dsize)
                return n - d2.dsize;
        if (n == 0)
                return 0;
        return memcmp(d1.dptr, d2.dptr, n);
}

static short
additem(char buf[PBLKSIZ], datum item)
{
        short *sp;
        int i1, i2;

        sp = (short *)buf;
        i1 = PBLKSIZ;
        if (sp[0] > 0)
                i1 = sp[sp[0] + 1 - 1];
        i1 -= item.dsize;
        i2 = (sp[0] + 2) * sizeof(short);
        if (i1 <= i2)
                return -1;
        sp[sp[0] + 1] = i1;
        for (i2 = 0; i2 < item.dsize; i2++) {
                buf[i1] = item.dptr[i2];
                i1++;
        }
        sp[0]++;
        return sp[0] - 1;
}

static void
delitem(char buf[PBLKSIZ], int n)
{
        short *sp;
        int i1, i2, i3;

        sp = (short *)buf;
        if (n < 0 || n >= sp[0])
                goto bad;
        i1 = sp[n + 1];
        i2 = PBLKSIZ;
        if (n > 0)
                i2 = sp[n + 1 - 1];
        i3 = sp[sp[0] + 1 - 1];
        if (i2 > i1)
        while (i1 > i3) {
                i1--;
                i2--;
                buf[i2] = buf[i1];
                buf[i1] = 0;
        }
        i2 -= i1;
        for (i1 = n + 1; i1 < sp[0]; i1++)
                sp[i1 + 1 - 1] = sp[i1 + 1] + i2;
        sp[0]--;
        sp[sp[0] + 1] = 0;
        return;

bad:
        printf("bad delitem\n");
        abort();
}

static datum
makdatum(char buf[PBLKSIZ], int n)
{
        short *sp;
        int t;
        datum item;

        sp = (short *)buf;
        if (n < 0 || n >= sp[0])
                goto null;
        t = PBLKSIZ;
        if (n > 0)
                t = sp[n + 1 - 1];
        item.dptr = buf+sp[n + 1];
        item.dsize = t - sp[n + 1];
        return item;

null:
        item.dptr = NULL;
        item.dsize = 0;
        return item;
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
        db->pagf = open(db->pagbuf, 2);
        if (db->pagf < 0)
                goto epagf;

        strcpy(db->pagbuf, file);
        strcat(db->pagbuf, ".dir");
        db->dirf = open(db->pagbuf, 2);
        if (db->dirf < 0)
                goto edirf;

        fstat(db->dirf, &statb);
        db->maxbno = statb.st_size * BYTESIZ - 1;
        db->access_oldb = -1;
        db->getbit_oldb = -1;
        return db;

        /* close db->dirf if more err proc. before this */
edirf:
        close(db->pagf);
epagf:
        free(db);
emalloc:
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
                                printf("items not in pairs\n");
                        return item;
                }
        }
}

int
delete(Database *db, datum key)
{
        int i;
        datum item;

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        return -1;
                if (cmpdatum(key, item) == 0) {
                        delitem(db->pagbuf, i);
                        delitem(db->pagbuf, i);
                        break;
                }
        }
        lseek(db->pagf, db->blkno * PBLKSIZ, 0);
        write(db->pagf, db->pagbuf, PBLKSIZ);
        return 0;
}

void
store(Database *db, datum key, datum dat)
{
        int i;
        datum item;
        char ovfbuf[PBLKSIZ];

loop:
        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (cmpdatum(key, item) == 0) {
                        delitem(db->pagbuf, i);
                        delitem(db->pagbuf, i);
                        break;
                }
        }
        i = additem(db->pagbuf, key);
        if (i < 0)
                goto split;
        if (additem(db->pagbuf, dat) < 0) {
                delitem(db->pagbuf, i);
                goto split;
        }
        lseek(db->pagf, db->blkno * PBLKSIZ, 0);
        write(db->pagf, db->pagbuf, PBLKSIZ);
        return;

split:
        if (key.dsize + dat.dsize + 2 * sizeof(short) >= PBLKSIZ) {
                printf("entry too big\n");
                return;
        }
        clearbuf(ovfbuf, PBLKSIZ);
        for (i = 0;;) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (calchash(item) & (db->hmask + 1)) {
                        additem(ovfbuf, item);
                        delitem(db->pagbuf, i);
                        item = makdatum(db->pagbuf, i);
                        if (item.dptr == NULL) {
                                printf("split not paired\n");
                                break;
                        }
                        additem(ovfbuf, item);
                        delitem(db->pagbuf, i);
                        continue;
                }
                i += 2;
        }
        lseek(db->pagf, db->blkno * PBLKSIZ, 0);
        write(db->pagf, db->pagbuf, PBLKSIZ);
        lseek(db->pagf, (db->blkno + db->hmask + 1) * PBLKSIZ, 0);
        write(db->pagf, ovfbuf, PBLKSIZ);
        setbit(db);
        goto loop;
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
