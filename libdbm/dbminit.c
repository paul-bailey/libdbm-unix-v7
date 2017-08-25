#include "dbm_priv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

static int
count_bits(unsigned long v)
{
        /*
         * We checked that it was no more than a certain value,
         * which means we don't have to worry about the width of
         * an unsigned long.
         *
         * Use the 32-bit count algorithm in case we change our
         * minds later about the value of MAXBLKSIZ.
         */
        v = (v & 0x55555555U) + ((v >> 1) & 0x55555555U);
        v = (v & 0x33333333U) + ((v >> 2) & 0x33333333U);
        v = (v & 0x0F0F0F0FU) + ((v >> 4) & 0x0F0F0F0FU);
        v = (v & 0x00FF00FFU) + ((v >> 8) & 0x00FF00FFU);
        v = (v & 0x0000FFFFU) + ((v >> 16) & 0x0000FFFFU);
        return v;
}

static int
count_shift(unsigned long v)
{
        int count = 0;
        while ((v & 01) == 0 && count < 32) {
                ++count;
                v >>= 1;
        }
        return count;
}

static long
parse_value(const char *svalue)
{
        char *endptr;
        unsigned long v;

        errno = 0;
        v = strtoul(svalue, &endptr, 0);
        if (errno || endptr == svalue)
                return -1;
        /*
         * Make sure this is a power of two (IE only one bit set)
         * and it is not unreasonably large.
         */
        if (v < MINBLKSIZ || v > MAXBLKSIZ)
                return -1;

        if (count_bits(v) != 1)
                return -1;

        return (long)v;
}

static char *
slide(const char *s)
{
        while (isblank((int)*s))
                ++s;
        return (char *)s;
}

static int
iseol(int c)
{
        return c == '\0' || c == '\r' || c == '\n';
}

static int
parse_params(Database *db, FILE *fp)
{
        long pblksiz = -1;
        long dblksiz = -1;
        char *line = NULL;
        size_t len = 0;
        int res = -1;

        while (getline(&line, &len, fp) != -1) {
                long *pval;
                char *eol;
                char *value;
                char *key;

                key = slide(line);

                eol = strchr(key, '#');
                if (eol != NULL)
                        *eol = '\0';

                /* Skip blank or comment-only lines */
                if (iseol(*key))
                        continue;

                /*
                 * Line is not blank, not a comment.
                 * It should have "KEY = VALUE" syntax.
                 */
                value = strchr(key, '=');
                if (value == NULL)
                        goto out;
                value = slide(value + 1);

                /*
                 * Shortcut our syntax parsing, since so far there are
                 * only two keys to check for.
                 * Don't bail on unidentified keys, so long as the syntax
                 * is okay; we may add keys in the future.
                 */
                if (*key == 'P')
                        pval = &pblksiz;
                else if (*key == 'D')
                        pval = &dblksiz;
                else
                        continue;
                ++key;

                if (!!strncmp("BLKSIZ", key, strlen("BLKSIZ")))
                        continue;

                if ((*pval = parse_value(value)) < 0)
                        goto out;
        }

        if (pblksiz < 0 || dblksiz < 0)
                goto out;

        db->pblksiz = pblksiz;
        db->dblksiz = dblksiz;
        res = 0;

out:
        if (line != NULL)
                free(line);
        return res;
}

static int
getparams(Database *db, const char *fname)
{
        FILE *fp;
        int res;
        fp = fopen(fname, "r");
        if (fp == NULL) {
                /* New DB: Create with defaults */
                fp = fopen(fname, "w");
                if (fp == NULL)
                        return -1;
                db->pblksiz = PBLKSIZ;
                db->dblksiz = DBLKSIZ;

                fprintf(fp, "%s\nPBLKSIZ=%u\nDBLKSIZ=%u\n",
                        "# Auto-generated. Do not manually edit or you will break the database!",
                        PBLKSIZ, DBLKSIZ);
                res = 0;
        } else {
                /* Existing DB: parse params */
                res = parse_params(db, fp);
                fclose(fp);
        }
        return res;
}

Database EXPORT*
dbminit(const char *file)
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

        sprintf(fname, "%s.par", file);
        if (getparams(db, fname) < 0)
                goto eparams;

        sprintf(fname, "%s.pag", file);
        db->pagfd = open(fname, O_CREAT | O_RDWR, 0666);
        if (db->pagfd < 0)
                goto epagfd;

        sprintf(fname, "%s.dir", file);
        db->dirfd = open(fname, O_CREAT | O_RDWR, 0666);
        if (db->dirfd < 0)
                goto edirfd;

        fstat(db->dirfd, &statb);
        db->maxbno = statb.st_size * BYTESIZ - 1;
        db->access_oldb = -1;
        db->getbit_oldb = -1;
        db->dblkshift = count_shift(db->dblksiz);
        db->pblkshift = count_shift(db->pblksiz);
        return db;

        /* close db->dirfd if more err proc. before this */
edirfd:
        close(db->pagfd);
epagfd:
eparams:
        free(db);
emalloc:
        DBG("cannot init DBM\n");
        return NULL;
}
