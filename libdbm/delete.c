#include "dbm_priv.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

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

int EXPORT
delete(Database *db, datum key)
{
        int i;
        datum item;

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        return -1;
                if (cmpdatum(key, item) == 0) {
                        delitem(db->pagbuf, i);
                        delitem(db->pagbuf, i);
                        break;
                }
        }
        lseek(db->pagfd, db->blkno * PBLKSIZ, 0);
        write(db->pagfd, db->pagbuf, PBLKSIZ);
        return 0;
}
