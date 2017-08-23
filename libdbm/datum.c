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

datum
makdatum(char buf[PBLKSIZ], int n)
{
        short *sp;
        int t;
        datum item;

        sp = (short *)buf;
        if (n < 0 || n >= sp[0])
                goto nullate;
        t = PBLKSIZ;
        if (n > 0)
                t = sp[n + 1 - 1];
        item.dptr = buf+sp[n + 1];
        item.dsize = t - sp[n + 1];
        return item;

nullate:
        item.dptr = NULL;
        item.dsize = 0;
        return item;
}
