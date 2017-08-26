#include "dbm_priv.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Make sure @buf is a valid block, panic otherwise */
static void
check_block(char *buf, size_t bufsize)
{
        short *sp;
        int t, i;

        sp = (short *)buf;
        t = bufsize;
        for (i = 0; i < sp[0]; i++) {
                if (sp[i + 1] > t)
                        goto bad;
                t = sp[i + 1];
        }
        if (t < (sp[0] + 1) * sizeof(short))
                goto bad;
        return;

bad:
        DBG("bad block\n");
        abort();
        memset(buf, 0, bufsize);
}

/*
 * Refill @db's page buffer with page containing data whose key yields
 * @hash, unless that is already the current page in the buffer.
 */
void
dbm_access(Database *db, long hash)
{
        forder_hash(db, hash);

        if (db->blkno != db->access_oldb) {
                int count;
                memset(db->pagbuf, 0, db->pblksiz);
                lseek(db->pagfd, db->blkno * db->pblksiz, 0);
                count = read(db->pagfd, db->pagbuf, db->pblksiz);
                (void)count;
                check_block(db->pagbuf, db->pblksiz);
                db->access_oldb = db->blkno;
        }
}
