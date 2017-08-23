#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

Database *
dbminit(char *file)
{
        struct stat statb;
        Database *db;

        db = malloc(sizeof(*db));
        if (!db)
                goto emalloc;

        memset(db, 0, sizeof(*db));
        strcpy(db->pagbuf, file);
        strcat(db->pagbuf, ".pag");
        db->pagfd = open(db->pagbuf, 2);
        if (db->pagfd < 0)
                goto epagfd;

        strcpy(db->pagbuf, file);
        strcat(db->pagbuf, ".dir");
        db->dirfd = open(db->pagbuf, 2);
        if (db->dirfd < 0)
                goto edirfd;

        fstat(db->dirfd, &statb);
        db->maxbno = statb.st_size * BYTESIZ - 1;
        db->access_oldb = -1;
        db->getbit_oldb = -1;
        return db;

        /* close db->dirfd if more err proc. before this */
edirfd:
        close(db->pagfd);
epagfd:
        free(db);
emalloc:
        fprintf(stderr, "cannot init DBM\n");
        return NULL;
}
