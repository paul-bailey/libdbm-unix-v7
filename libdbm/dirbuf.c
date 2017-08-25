#include "dbm_priv.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Used for breaking a big bit number down to block-byte-bit */
struct bitidx_t {
        int i; /* Byte no. within block */
        int b; /* Block no. */
        int n; /* Bit no. within byte */
};

/* helper to getbit() and setbit() */
static void
fill_bitidx(Database *db, struct bitidx_t *b)
{
        long bn;
        long bitno = db->bitno;
        b->n = bitno % BYTESIZ;
        bn = bitno / BYTESIZ;
        b->i = bn & (db->dblksiz - 1);
        b->b = bn >> db->dblkshift;
}

int
getbit(Database *db)
{
        struct bitidx_t b;

        if (db->bitno > db->maxbno)
                return 0;

        fill_bitidx(db, &b);
        if (b.b != db->getbit_oldb) {
                int count;
                memset(db->dirbuf, 0, db->dblksiz);
                lseek(db->dirfd, (long)b.b * db->dblksiz, 0);
                count = read(db->dirfd, db->dirbuf, db->dblksiz);
                (void)count;
                db->getbit_oldb = b.b;
        }

        return !!(db->dirbuf[b.i] & (1 << b.n));
}

void
setbit(Database *db)
{
        struct bitidx_t b;
        int count;

        if (db->bitno > db->maxbno) {
                db->maxbno = db->bitno;
                getbit(db);
        }
        fill_bitidx(db, &b);

        db->dirbuf[b.i] |= 1 << b.n;
        lseek(db->dirfd, (long)b.b * db->dblksiz, 0);
        count = write(db->dirfd, db->dirbuf, db->dblksiz);
        (void)count;
}
