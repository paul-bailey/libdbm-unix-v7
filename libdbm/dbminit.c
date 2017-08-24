#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

Database EXPORT*
dbminit(char *file)
{
        struct stat statb;
        Database *db;
        char *fname;

        /* TODO: Stack protection */
        fname = alloca(strlen(file) + 5);

        db = malloc(sizeof(*db));
        if (!db)
                goto emalloc;

        memset(db, 0, sizeof(*db));
        sprintf(fname, "%s.pag", file);
        db->pagfd = open(fname, O_CREAT | O_RDWR);
        if (db->pagfd < 0)
                goto epagfd;

        sprintf(fname, "%s.dir", file);
        db->dirfd = open(fname, O_CREAT | O_RDWR);
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
        DBG("cannot init DBM\n");
        return NULL;
}
