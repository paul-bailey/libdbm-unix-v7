#include "dbm_priv.h"
#include <stdio.h>

/**
 * fetch - Retrieve datum that was previously stored
 * @db: Database to retrieve datum from
 * @key: The key corresponding to the datum that was stored
 *
 * Return: A datum type.  If @key was not found in @db, then the
 * .dptr field will be NULL.
 *
 * WARNING: Treat the data pointed at by the return value's .dptr field
 * as read-only.
 */
datum EXPORT
fetch(Database *db, datum key)
{
        int i;
        datum item;

        DBG("Gonna access '%s'\n", key.dptr);
        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                DBG("i=%d, gonna find item\n", i);
                item = makdatum(db->pagbuf, i, db->pblksiz);
                if (item.dptr == NULL) {
                        DBG("item not found\n");
                        return item;
                }

                if (cmpdatum(key, item) == 0) {
                        item = makdatum(db->pagbuf, i + 1, db->pblksiz);
                        if (item.dptr == NULL)
                                DBG("items not in pairs\n");
                        return item;
                }
        }
}
