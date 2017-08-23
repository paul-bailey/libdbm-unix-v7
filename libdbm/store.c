#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

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

static int store_r(Database *db, datum key, datum dat, char ovfbuf[PBLKSIZ]);
static int
store_r_helper(Database *db, datum key, datum dat, char ovfbuf[PBLKSIZ])
{
        int i;

        if (key.dsize + dat.dsize + 2 * sizeof(short) >= PBLKSIZ) {
                DBG("entry too big\n");
                return -1;
        }

        memset(ovfbuf, 0, PBLKSIZ);
        for (i = 0;;) {
                datum item;
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (!!(calchash(item) & (db->hmask + 1))) {
                        additem(ovfbuf, item);
                        delitem(db->pagbuf, i);
                        item = makdatum(db->pagbuf, i);
                        if (item.dptr == NULL) {
                                DBG("split not paired\n");
                                break;
                        }
                        additem(ovfbuf, item);
                        delitem(db->pagbuf, i);
                        continue;
                }
                i += 2;
        }
        lseek(db->pagfd, db->blkno * PBLKSIZ, 0);
        write(db->pagfd, db->pagbuf, PBLKSIZ);
        lseek(db->pagfd, (db->blkno + db->hmask + 1) * PBLKSIZ, 0);
        write(db->pagfd, ovfbuf, PBLKSIZ);
        setbit(db);

        /* now recursively try again */
        return store_r(db, key, dat, ovfbuf);
}

static int
store_r(Database *db, datum key, datum dat, char ovfbuf[PBLKSIZ])
{
        int i;
        int keyi, datai;

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                datum item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (cmpdatum(key, item) == 0) {
                        delitem(db->pagbuf, i);
                        delitem(db->pagbuf, i);
                        break;
                }
        }

        keyi = additem(db->pagbuf, key);
        if (i < 0)
                goto ekey;

        datai = additem(db->pagbuf, dat);
        if (datai < 0)
                goto edat;

        lseek(db->pagfd, db->blkno * PBLKSIZ, 0);
        write(db->pagfd, db->pagbuf, PBLKSIZ);
        return 0;

edat:
        delitem(db->pagbuf, keyi);
ekey:
        return store_r_helper(db, key, dat, ovfbuf);
}

int EXPORT
store(Database *db, datum key, datum dat)
{
        /*
         * Declared here because otherwise it may keep filling up the
         * stack, even though each new declaration occurs after the
         * older instance is no longer used.
         */
        char ovfbuf[PBLKSIZ];
        return store_r(db, key, dat, ovfbuf);
}
