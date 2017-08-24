#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

static short
additem(char buf[PBLKSIZ], datum item)
{
        short *sp;
        int i;

        sp = (short *)buf;
        i = sp[0] > 0 ? sp[sp[0]] : PBLKSIZ;
        i -= item.dsize;

        /* Make sure we can fit index no. and data */
        if (i <= ((sp[0] + 2) * sizeof(short)))
                return -1;

        /* Store index number */
        sp[sp[0] + 1] = i;

        /* Store data */
        memcpy(&buf[i], item.dptr, item.dsize);

        /* Update index count */
        sp[0]++;

        /* Return old index count. */
        return sp[0] - 1;
}

static int
store_helper(Database *db, datum key, datum dat)
{
        int i;
        char ovfbuf[PBLKSIZ];

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
        return 0;
}

int EXPORT
store(Database *db, datum key, datum dat)
{
        int i;
        int keyi, datai;

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                datum item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        break;
                if (cmpdatum(key, item) == 0) {
                        /* TODO: Is this a collision? */
                        delitem(db->pagbuf, i);
                        delitem(db->pagbuf, i);
                        break;
                }
        }

        keyi = additem(db->pagbuf, key);
        if (keyi < 0)
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
        if (store_helper(db, key, dat) < 0)
                return -1;

        return store(db, key, dat);
}
