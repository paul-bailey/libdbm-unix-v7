#include "dbm_priv.h"
#include <stdio.h>

datum EXPORT
fetch(Database *db, datum key)
{
        int i;
        datum item;

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        return item;
                if (cmpdatum(key, item) == 0) {
                        item = makdatum(db->pagbuf, i + 1);
                        if (item.dptr == NULL)
                                fprintf(stderr, "items not in pairs\n");
                        return item;
                }
        }
}
