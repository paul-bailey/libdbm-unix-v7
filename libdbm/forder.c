#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Find which block number/bit number contains data whose key yields
 * @hash.
 *
 * dbm_access() and forder() call this to set db->hmask, db->bitno, and
 * db->blkno, which are referenced by their calling functions.
 */
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

/* Make sure @buf is a valid block, panic otherwise */
static void
check_block(char buf[PBLKSIZ])
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
        DBG("bad block\n");
        abort();
        memset(buf, 0, PBLKSIZ);
}

/*
 * Refill @db's page buffer with page containing data whose key yields
 * @hash, unless that is already the current page in the buffer.
 */
void
dbm_access(Database *db, long hash)
{
        hmask_cycle(db, hash);

        if (db->blkno != db->access_oldb) {
                memset(db->pagbuf, 0, PBLKSIZ);
                lseek(db->pagfd, db->blkno * PBLKSIZ, 0);
                read(db->pagfd, db->pagbuf, PBLKSIZ);
                check_block(db->pagbuf);
                db->access_oldb = db->blkno;
        }
}

/*
 * Get the block number of datum whose key is @key
 */
long EXPORT
forder(Database *db, datum key)
{
        /* TODO: A version of this that doesn't mess up db local vars? */
        hmask_cycle(db, calchash(key));
        return db->blkno;
}
