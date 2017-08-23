#ifndef DBM_H
# define DBM_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PBLKSIZ 512
#define DBLKSIZ 8192
#define BYTESIZ 8
#define NULL    ((char *) 0)

extern long bitno;
extern long maxbno;
extern long blkno;
extern long hmask;

extern char pagbuf[PBLKSIZ];
extern char dirbuf[DBLKSIZ];

extern int dirf;
extern int pagf;

typedef struct {
        char *dptr;
        int dsize;
} datum;

extern datum fetch(datum key);
extern datum makdatum(char buf[PBLKSIZ], int n);
extern datum firstkey(void);
extern datum nextkey(datum key);
extern datum firsthash(long hash);
extern long calchash(datum item);
extern long hashinc(long hash);

#ifdef __cplusplus
}
#endif

#endif /* DBM_H */
