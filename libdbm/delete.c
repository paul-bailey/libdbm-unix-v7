#include "dbm_priv.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void
delitem(char buf[PBLKSIZ], int n)
{
        short *sp;
        int i1, i2, i3;

        sp = (short *)buf;
        if (n < 0 || n >= sp[0])
                goto bad;
        i1 = sp[n + 1];
        i2 = PBLKSIZ;
        if (n > 0)
                i2 = sp[n + 1 - 1];
        i3 = sp[sp[0] + 1 - 1];
        if (i2 > i1) {
                while (i1 > i3) {
                        i1--;
                        i2--;
                        buf[i2] = buf[i1];
                        buf[i1] = 0;
                }
        }
        i2 -= i1;
        for (i1 = n + 1; i1 < sp[0]; i1++)
                sp[i1 + 1 - 1] = sp[i1 + 1] + i2;
        sp[0]--;
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
