#include "dbm_priv.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int EXPORT
delete(Database *db, datum key)
{
        int i, count;
        datum item;

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i, db->pblksiz);
                if (item.dptr == NULL)
                        return -1;
                if (cmpdatum(key, item) == 0) {
                        /*
                         * Delete both key and data.
                         * Do not update i here because
                         * delitem() shifts items over.
                         */
                        delitem(db->pagbuf, i, db->pblksiz);
                        delitem(db->pagbuf, i, db->pblksiz);
                        break;
                }
        }
        lseek(db->pagfd, db->blkno * db->pblksiz, 0);
        count = write(db->pagfd, db->pagbuf, db->pblksiz);
        (void)count;
        return 0;
}
