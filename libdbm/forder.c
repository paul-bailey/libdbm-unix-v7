#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * forder_hash - Helper to forder() and dbm_access().
 *
 * Find which block number/bit number contains data whose key yields
 * @hash.
 *
 * dbm_access() and forder() call this to set db->hmask, db->bitno, and
 * db->blkno, which are referenced by their calling functions.
 */
void
forder_hash(Database *db, long hash)
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

/*
 * Get the block number of datum whose key is @key
 */
long EXPORT
forder(Database *db, datum key)
{
        /* TODO: A version of this that doesn't mess up db local vars? */
        forder_hash(db, calchash(key));
        return db->blkno;
}
