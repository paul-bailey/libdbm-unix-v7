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
