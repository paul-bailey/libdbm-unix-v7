#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
        datum key, data;
        int res;
        Database *db;

        if (argc <= 2) {
                fprintf(stderr, "Expected: fetch KEY DATA\n");
                exit(EXIT_FAILURE);
        }

        key.dptr = argv[1];
        key.dsize = strlen(argv[1] + 1);
        data.dptr = argv[2];
        data.dsize = strlen(argv[2] + 1);

        db_set_path();
        db = dbminit(dbpath);
        if (!db) {
                fprintf(stderr, "dbminit() failed\n");
                exit(EXIT_FAILURE);
        }
        res = store(db, key, data);
        if (res < 0) {
                fprintf(stderr, "store() failed\n");
                exit(EXIT_FAILURE);
        }
        printf("Stored KEY=%s DAT=%s\n", key.dptr, data.dptr);
        return 0;
}
