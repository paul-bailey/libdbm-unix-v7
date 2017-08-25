#include "dbm_priv.h"
#include <string.h>
#include <stdlib.h>

/**
 * makdatum - Create a &datum type for data in buf whose index is idx
 * @buf: The database buffer containing the data
 * @idx: Visible index of datum in @buf
 */
datum
makdatum(char *buf, int idx, size_t pblksiz)
{
        short *sp;
        datum item;

        sp = (short *)buf;
        if (idx < 0 || idx >= sp[0]) {
                item.dptr = NULL;
                item.dsize = 0;
        } else {
                int t = pblksiz;
                if (idx > 0)
                        t = sp[idx];
                item.dptr = buf + sp[idx + 1];
                item.dsize = t - sp[idx + 1];
        }
        return item;
}

/**
 * delitem - Delete a datum stored in a buffer
 * @buf: Page buffer containing data to delete
 * @idx: Visible index of datum in @buf
 *
 * The program will abort if @idx is not a datum stored in @buf.
 */
void
delitem(char *buf, int idx, size_t pblksiz)
{
        short *sp;
        int datai, previ, botti, idxi;

        sp = (short *)buf;
        if (idx < 0 || idx >= sp[0])
                goto bad;

        datai = sp[idx + 1];
        previ = idx > 0 ? sp[idx] : pblksiz;
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

/**
 * Add a datum to a page buffer
 * @buf: Page buffer to store datum in
 * @item: Datum to copy into @buf
 *
 * Return: Visible index of datum in @buf.  If it's the first datum stored
 * in this buffer, the return value will be 0. If it's the second, the
 * return value will be 1, and so on... If there is not room left in the
 * buffer the return value will be -1.
 */
int
additem(char *buf, datum item, size_t pblksiz)
{
        short *sp;
        int i;

        sp = (short *)buf;
        i = sp[0] > 0 ? sp[sp[0]] : pblksiz;
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
