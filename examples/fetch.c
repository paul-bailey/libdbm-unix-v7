#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
        datum res, key;
        Database *db;

        if (argc <= 1) {
                fprintf(stderr, "Expected: fetch KEY\n");
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
        res = fetch(db, key);
        printf("%s\n", res.dptr);
        return 0;
}
