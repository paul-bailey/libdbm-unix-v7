#include "dbm_priv.h"
#include <string.h>

int
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

/**
 * makdatum - Create a &datum type for data in buf whose index is idx
 * @buf: The database buffer containing the data
 * @idx: The index of the data-index
 */
datum
makdatum(char buf[PBLKSIZ], int idx)
{
        short *sp;
        datum item;

        sp = (short *)buf;
        if (idx < 0 || idx >= sp[0]) {
                item.dptr = NULL;
                item.dsize = 0;
        } else {
                int t = PBLKSIZ;
                if (idx > 0)
                        t = sp[idx];
                item.dptr = buf + sp[idx + 1];
                item.dsize = t - sp[idx + 1];
        }
        return item;
}
