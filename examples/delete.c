#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
        datum key;
        int res;
        Database *db;

        if (argc <= 1) {
                fprintf(stderr, "Expected: delete KEY\n");
                exit(EXIT_FAILURE);
        }

        key.dptr = argv[1];
        key.dsize = strlen(argv[1]) + 1;

        db_set_path();
        db = dbminit(dbpath);
        if (!db) {
                fprintf(stderr, "dbminit() failed\n");
                exit(EXIT_FAILURE);
        }
        res = delete(db, key);
        if (res < 0) {
                fprintf(stderr, "delete() failed\n");
                exit(EXIT_FAILURE);
        }
        return 0;
}
