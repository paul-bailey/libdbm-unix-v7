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

        dbm_access(db, calchash(key));
        for (i = 0;; i += 2) {
                item = makdatum(db->pagbuf, i);
                if (item.dptr == NULL)
                        return item;
                if (cmpdatum(key, item) == 0) {
                        item = makdatum(db->pagbuf, i + 1);
                        if (item.dptr == NULL)
                                DBG("items not in pairs\n");
                        return item;
                }
        }
}
