#include "dbm_priv.h"
#include <unistd.h>
#include <stdlib.h>

/**
 * dbm_close - Close a database
 * @db: Database to close.  Do not use @db or dereference any data
 * returned from fetch() after calling this function.
 */
void EXPORT
dbm_close(Database *db)
{
        close(db->dirfd);
        close(db->pagfd);
        free(db);
}
