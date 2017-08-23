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

static struct Database db = {
        .access_oldb = -1,
        .getbit_oldb = -1,
};

static void
clearbuf(char *cp, int n)
{
        memset(cp, 0, n);
}

static int
getbit(void)
{
        long bn;
        int b, i, n;

        if (db.bitno > db.maxbno)
                return 0;

        n = db.bitno % BYTESIZ;
        bn = db.bitno / BYTESIZ;
        i = bn % DBLKSIZ;
        b = bn / DBLKSIZ;
        if (b != db.getbit_oldb) {
                clearbuf(db.dirbuf, DBLKSIZ);
                lseek(db.dirf, (long)b*DBLKSIZ, 0);
                read(db.dirf, db.dirbuf, DBLKSIZ);
                db.getbit_oldb = b;
        }

        if (db.dirbuf[i] & (1 << n))
                return 1;
        return 0;
}

static void
setbit(void)
{
        long bn;
        int i, n, b;

        if (db.bitno > db.maxbno) {
                db.maxbno = db.bitno;
                getbit();
        }
        n = db.bitno % BYTESIZ;
        bn = db.bitno / BYTESIZ;
        i = bn % DBLKSIZ;
        b = bn / DBLKSIZ;
        db.dirbuf[i] |= 1<<n;
        lseek(db.dirf, (long)b * DBLKSIZ, 0);
        write(db.dirf, db.dirbuf, DBLKSIZ);
}

static void
hmask_cycle(long hash)
{
        long hmask;
        for (hmask = 0;; hmask = (hmask << 1) + 1) {
                db.blkno = hash & hmask;
                db.bitno = db.blkno + hmask;
                if (getbit() == 0)
                        break;
        }
        db.hmask = hmask;
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
dbm_access(long hash)
{
        hmask_cycle(hash);

        if (db.blkno != db.access_oldb) {
                clearbuf(db.pagbuf, PBLKSIZ);
                lseek(db.pagf, db.blkno * PBLKSIZ, 0);
                read(db.pagf, db.pagbuf, PBLKSIZ);
                chkblk(db.pagbuf);
                db.access_oldb = db.blkno;
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
hashinc(long hash)
{
        long bit;

        hash &= db.hmask;
        bit = db.hmask + 1;
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
firsthash(long hash)
{
        int i;
        datum item, bitem;

        for (;;) {
                dbm_access(hash);
                bitem = makdatum(db.pagbuf, 0);
                for (i = 2;; i += 2) {
                        item = makdatum(db.pagbuf, i);
                        if (item.dptr == NULL)
                                break;
                        if (cmpdatum(bitem, item) < 0)
                                bitem = item;
                }
                if (bitem.dptr != NULL)
                        return bitem;
                hash = hashinc(hash);
                if (hash == 0)
                        return item;
        }
}

int
dbminit(char *file)
{
        struct stat statb;

        strcpy(db.pagbuf, file);
        strcat(db.pagbuf, ".pag");
        db.pagf = open(db.pagbuf, 2);

        strcpy(db.pagbuf, file);
        strcat(db.pagbuf, ".dir");
        db.dirf = open(db.pagbuf, 2);
        if (db.pagf < 0 || db.dirf < 0) {
                printf("cannot open database %s\n", file);
                return -1;
        }
        fstat(db.dirf, &statb);
        db.maxbno = statb.st_size * BYTESIZ - 1;
        return 0;
}

long
forder(datum key)
{
        hmask_cycle(calchash(key));
        return db.blkno;
}

datum
fetch(datum key)
{
        int i;
        datum item;

        dbm_access(calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db.pagbuf, i);
                if (item.dptr == NULL)
                        return item;
                if (cmpdatum(key, item) == 0) {
                        item = makdatum(db.pagbuf, i + 1);
                        if (item.dptr == NULL)
                                printf("items not in pairs\n");
                        return item;
                }
        }
}

int
delete(datum key)
{
        int i;
        datum item;

        dbm_access(calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db.pagbuf, i);
                if (item.dptr == NULL)
                        return -1;
                if (cmpdatum(key, item) == 0) {
                        delitem(db.pagbuf, i);
                        delitem(db.pagbuf, i);
                        break;
                }
        }
        lseek(db.pagf, db.blkno * PBLKSIZ, 0);
        write(db.pagf, db.pagbuf, PBLKSIZ);
        return 0;
}

void
store(datum key, datum dat)
{
        int i;
        datum item;
        char ovfbuf[PBLKSIZ];

loop:
        dbm_access(calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db.pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (cmpdatum(key, item) == 0) {
                        delitem(db.pagbuf, i);
                        delitem(db.pagbuf, i);
                        break;
                }
        }
        i = additem(db.pagbuf, key);
        if (i < 0)
                goto split;
        if (additem(db.pagbuf, dat) < 0) {
                delitem(db.pagbuf, i);
                goto split;
        }
        lseek(db.pagf, db.blkno * PBLKSIZ, 0);
        write(db.pagf, db.pagbuf, PBLKSIZ);
        return;

split:
        if (key.dsize + dat.dsize + 2 * sizeof(short) >= PBLKSIZ) {
                printf("entry too big\n");
                return;
        }
        clearbuf(ovfbuf, PBLKSIZ);
        for (i = 0;;) {
                item = makdatum(db.pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (calchash(item) & (db.hmask + 1)) {
                        additem(ovfbuf, item);
                        delitem(db.pagbuf, i);
                        item = makdatum(db.pagbuf, i);
                        if (item.dptr == NULL) {
                                printf("split not paired\n");
                                break;
                        }
                        additem(ovfbuf, item);
                        delitem(db.pagbuf, i);
                        continue;
                }
                i += 2;
        }
        lseek(db.pagf, db.blkno * PBLKSIZ, 0);
        write(db.pagf, db.pagbuf, PBLKSIZ);
        lseek(db.pagf, (db.blkno + db.hmask + 1) * PBLKSIZ, 0);
        write(db.pagf, ovfbuf, PBLKSIZ);
        setbit();
        goto loop;
}

datum
firstkey(void)
{
        return firsthash(0L);
}

datum
nextkey(datum key)
{
        int i;
        datum item, bitem;
        long hash;
        int f;

        hash = calchash(key);
        dbm_access(hash);
        f = 1;
        for (i = 0;; i += 2) {
                item = makdatum(db.pagbuf, i);
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
        hash = hashinc(hash);
        if (hash == 0)
                return item;
        return firsthash(hash);
}
