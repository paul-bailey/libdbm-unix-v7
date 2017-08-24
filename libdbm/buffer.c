#include "dbm_priv.h"
#include <string.h>
#include <stdlib.h>

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

void
delitem(char buf[PBLKSIZ], int idx)
{
        short *sp;
        int datai, previ, botti, idxi;

        sp = (short *)buf;
        if (idx < 0 || idx >= sp[0])
                goto bad;

        datai = sp[idx + 1];
        previ = idx > 0 ? sp[idx] : PBLKSIZ;
        botti = sp[sp[0]];

        /* Maybe move data to fill in hole. */
        if (previ > datai) {
                while (datai > botti) {
                        datai--;
                        previ--;
                        buf[previ] = buf[datai];
                        buf[datai] = 0;
                }
        }
        previ -= datai;

        /* Update shift higher indices to fill in removed index */
        for (idxi = idx + 1; idxi < sp[0]; idxi++)
                sp[idxi] = sp[idxi + 1] + previ;

        /* Update index count */
        sp[0]--;

        /* Clear old index number */
        sp[sp[0] + 1] = 0;
        return;

bad:
        DBG("bad delitem\n");
        abort();
}

int
additem(char buf[PBLKSIZ], datum item)
{
        short *sp;
        int i;

        sp = (short *)buf;
        i = sp[0] > 0 ? sp[sp[0]] : PBLKSIZ;
        i -= item.dsize;

        /* Make sure we can fit index no. and data */
        if (i <= ((sp[0] + 2) * sizeof(short)))
                return -1;

        /* Store index number */
        sp[sp[0] + 1] = i;

        /* Store data */
        memcpy(&buf[i], item.dptr, item.dsize);

        /* Update index count */
        sp[0]++;

        /* Return old index count. */
        return sp[0] - 1;
}
