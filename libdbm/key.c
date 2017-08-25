#include "dbm_priv.h"
#include <stddef.h>

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
                bitem = makdatum(db->pagbuf, 0, db->pblksiz);
                for (i = 2;; i += 2) {
                        item = makdatum(db->pagbuf, i, db->pblksiz);
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

datum EXPORT
firstkey(Database *db)
{
        return firsthash(db, 0L);
}

datum EXPORT
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
                item = makdatum(db->pagbuf, i, db->pblksiz);
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
