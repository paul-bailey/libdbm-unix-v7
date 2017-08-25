#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* Create a new page in the database */
static int
store_helper(Database *db, datum key, datum dat)
{
        int i, count;
        char *ovfbuf;

        if (key.dsize + dat.dsize + 3 * sizeof(short) >= db->pblksiz) {
                DBG("entry too big\n");
                return -1;
        }

        ovfbuf = malloc(db->pblksiz);
        if (!ovfbuf) {
                DBG("Out of memory\n");
                return -1;
        }

        memset(ovfbuf, 0, db->pblksiz);
        for (i = 0;;) {
                datum item = makdatum(db->pagbuf, i, db->pblksiz);
                if (item.dptr == NULL)
                        break;
                if (!!(calchash(item) & (db->hmask + 1))) {
                        additem(ovfbuf, item, db->pblksiz);
                        delitem(db->pagbuf, i, db->pblksiz);
                        item = makdatum(db->pagbuf, i, db->pblksiz);
                        if (item.dptr == NULL) {
                                DBG("split not paired\n");
                                break;
                        }
                        additem(ovfbuf, item, db->pblksiz);
                        delitem(db->pagbuf, i, db->pblksiz);
                        continue;
                }
                i += 2;
        }

        lseek(db->pagfd, db->blkno * db->pblksiz, 0);
        count = write(db->pagfd, db->pagbuf, db->pblksiz);
        (void)count;
        lseek(db->pagfd, (db->blkno + db->hmask + 1) * db->pblksiz, 0);
        count = write(db->pagfd, ovfbuf, db->pblksiz);
        (void)count;
        free(ovfbuf);

        setbit(db);

        /* now recursively try again */
        return 0;
}

/**
 * store - Store key/data pair
 *
 * Return: 0 if stored successfully, -1 if the entry is too big.
 */
int EXPORT
store(Database *db, datum key, datum dat)
{
        int i, count;
        int keyi, datai;

        dbm_access(db, calchash(key));

        for (i = 0;; i += 2) {
                datum item = makdatum(db->pagbuf, i, db->pblksiz);
                if (item.dptr == NULL)
                        break;
                if (cmpdatum(key, item) == 0) {
                        /* TODO: Is this a collision? */
                        delitem(db->pagbuf, i, db->pblksiz);
                        delitem(db->pagbuf, i, db->pblksiz);
                        break;
                }
        }

        keyi = additem(db->pagbuf, key, db->pblksiz);
        if (keyi < 0)
                goto ekey;

        datai = additem(db->pagbuf, dat, db->pblksiz);
        if (datai < 0)
                goto edat;

        lseek(db->pagfd, db->blkno * db->pblksiz, 0);
        count = write(db->pagfd, db->pagbuf, db->pblksiz);
        (void)count;
        return 0;

edat:
        delitem(db->pagbuf, keyi, db->pblksiz);
ekey:
        if (store_helper(db, key, dat) < 0)
                return -1;

        return store(db, key, dat);
}
