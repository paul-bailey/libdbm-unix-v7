#ifndef DBM_H
# define DBM_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Tunable parameters */
enum {
        PBLKSIZ = 512,
        DBLKSIZ = 8192,
        BYTESIZ = 8,
};

/* Options - some are preprocessed, so they can't be enums */
#define DBG_MESSAGING 0

typedef struct {
        char *dptr;
        int dsize;
} datum;

/* FIXME: Make this type threadsafe. Currently it is not. */
typedef struct Database Database;

extern Database *dbminit(const char *file);
extern long forder(Database *db, datum key);
extern datum fetch(Database *db, datum key);
extern int delete(Database *db, datum key);
extern int store(Database *db, datum key, datum dat);
extern datum firstkey(Database *db);
extern datum nextkey(Database *db, datum key);
extern void dbm_close(Database *db);

extern long calchash(datum item);

#ifdef __cplusplus
}
#endif

#endif /* DBM_H */
