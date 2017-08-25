#include "common.h"
#include "../libdbm/dbm_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
        datum key;
        Database *db;

        db_set_path();
        db = dbminit(dbpath);
        if (!db) {
                fprintf(stderr, "dbminit() failed\n");
                exit(EXIT_FAILURE);
        }

        for (key = firstkey(db); key.dptr != NULL; key = nextkey(db, key)) {
                datum dat = fetch(db, key);
                if (dat.dptr == NULL) {
                        fprintf(stderr, "key '%s' not found!\n", key.dptr);
                        exit(EXIT_FAILURE);
                }
                /*
                 * This assumes that db had data stored using the
                 * methods in this local directory, wherein the
                 * terminating nul-char of a string was included
                 * in the store() and fetch() methods.
                 */
                printf("key: '%s'\n", key.dptr);
                printf("\tdat: '%s'\n", dat.dptr);
                printf("\torder: %lu\n", forder(db, key));
                printf("\thash:  %lu\n", calchash(key));
        }
        printf("db->maxbno = %lu\n", db->maxbno);
        dbm_close(db);
        return 0;
}
