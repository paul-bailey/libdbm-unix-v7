#ifndef DBM_H
# define DBM_H

#ifdef __cplusplus
extern "C"
{
#endif

enum {
        PBLKSIZ = 512,
        DBLKSIZ = 8192,
        BYTESIZ = 8,
};

typedef struct {
        char *dptr;
        int dsize;
} datum;

extern int dbminit(char *file);
extern long forder(datum key);
extern datum fetch(datum key);
extern int delete(datum key);
extern void store(datum key, datum dat);
extern datum firstkey(void);
extern datum nextkey(datum key);

extern long calchash(datum item);

#ifdef __cplusplus
}
#endif

#endif /* DBM_H */
