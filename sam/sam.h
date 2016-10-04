/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include "errors.h"

/*
 * BLOCKSIZE is relatively small to keep memory consumption down.
 */

#define BLOCKSIZE   2048
#define RUNESIZE    sizeof(wchar_t)
#define NDISC       5
#define NBUFFILES   3+2*NDISC   /* plan 9+undo+snarf+NDISC*(transcript+buf) */
#define NSUBEXP 10

#define INFINITY    0x7FFFFFFFL
#define INCR        25
#define STRSIZE     (2*BLOCKSIZE)

typedef int64_t        Posn;       /* file position or address */
typedef uint64_t      Mod;        /* modification number */

typedef struct Address  Address;
typedef struct Block    Block;
typedef struct Buffer   Buffer;
typedef struct File File;
typedef struct List List;
typedef struct Mark Mark;
typedef struct Range    Range;
typedef struct Rangeset Rangeset;
typedef struct String   String;

enum State
{
    Clean =     ' ',
    Dirty =     '\'',
    Unread =    '-',
    Readerr =   '~'
};

struct Range
{
    Posn    p1, p2;
};

struct Rangeset
{
    Range   p[NSUBEXP];
};

struct Address
{
    Range   r;
    File    *f;
};

struct List /* code depends on a int64_t being able to hold a pointer */
{
    int nalloc;
    int nused;
    union{
        void    *listp;
        Block   *blkp;
        int64_t    *longp;
        uint8_t*  *uint8_tp;
        String* *stringp;
        File*   *filep;
        int64_t    listv;
    }g;
};

#define listptr     g.listp
#define blkptr      g.blkp
#define longptr     g.longp
#define uint8_tpptr   g.uint8_tp
#define stringpptr  g.stringp
#define filepptr    g.filep
#define listval     g.listv

struct String
{
    int16_t   n;
    int16_t   size;
    wchar_t    *s;
};

struct Gapbuffer;
struct Buffer
{
    Posn nrunes;
    struct Gapbuffer *gb;
};

#define NGETC   128

struct File
{
    Buffer  *buf;       /* cached disc storage */
    Buffer  *transcript;    /* what's been done */
    Posn    markp;      /* file pointer to start of latest change */
    Mod mod;        /* modification stamp */
    Posn    nrunes;     /* total length of file */
    Posn    hiposn;     /* highest address touched this Mod */
    Address dot;        /* current position */
    Address ndot;       /* new current position after update */
    Range   tdot;       /* what terminal thinks is current range */
    Range   mark;       /* tagged spot in text (don't confuse with Mark) */
    List    *rasp;      /* map of what terminal's got */
    String  name;       /* file name */
    int16_t   tag;        /* for communicating with terminal */
    char    state;      /* Clean, Dirty, Unread, or Readerr*/
    char    closeok;    /* ok to close file? */
    char    deleted;    /* delete at completion of command */
    bool    marked;     /* file has been Fmarked at least once; once
                 * set, this will never go off as undo doesn't
                 * revert to the dawn of time */
    int64_t    dev;        /* file system from which it was read */
    int64_t    qid;        /* file from which it was read */
    int64_t    date;       /* time stamp of plan9 file */
    Posn    cp1, cp2;   /* Write-behind cache positions and */
    String  cache;      /* string */
    wchar_t    getcbuf[NGETC];
    int ngetc;
    int getci;
    Posn    getcp;
};

struct Mark
{
    Posn    p;
    Range   dot;
    Range   mark;
    Mod m;
    int16_t   s1;
};

/*
 * The precedent to any message in the transcript.
 * The component structures must be an integral number of Runes long.
 */
union Hdr
{
    struct _csl
    {
        int16_t   c;
        int16_t   s;
        int64_t    l;
    }csl;
    struct _cs
    {
        int16_t   c;
        int16_t   s;
    }cs;
    struct _cll
    {
        int16_t   c;
        int64_t    l;
        int64_t    l1;
    }cll;
    Mark    mark;
};

#define Fgetc(f)  ((--(f)->ngetc<0)? Fgetcload(f, (f)->getcp) : (f)->getcbuf[(f)->getcp++, (f)->getci++])
#define Fbgetc(f) (((f)->getci<=0)? Fbgetcload(f, (f)->getcp) : (f)->getcbuf[--(f)->getcp, --(f)->getci])

void    Bterm(Buffer*);
void    Bdelete(Buffer*, Posn, Posn);
void    Bflush(Buffer*);
void    Binsert(Buffer*, String*, Posn);
Buffer  *Bopen(void);
int Bread(Buffer*, wchar_t*, int, Posn);
int Fbgetcload(File*, Posn);
int Fbgetcset(File*, Posn);
int64_t    Fchars(File*, wchar_t*, Posn, Posn);
void    Fclose(File*);
void    Fdelete(File*, Posn, Posn);
int Fgetcload(File*, Posn);
int Fgetcset(File*, Posn);
void    Finsert(File*, String*, Posn);
File    *Fopen(void);
void    Fsetname(File*, String*);
void    Fstart(void);
int Fupdate(File*, int, int);
int Read(int, void*, int);
void    Seek(int, int64_t, int);
int plan9(File*, int, String*, int);
int Write(int, void*, int);
int bexecute(File*, Posn);
void    cd(String*);
void    closefiles(File*, String*);
void    closeio(Posn);
void    cmdloop(void);
void    cmdupdate(void);
void    compile(String*);
void    copy(File*, Address);
File    *current(File*);
void    delete(File*);
void    delfile(File*);
void    dellist(List*, int);
void    doubleclick(File*, Posn);
void    dprint(char*, ...);
void    edit(File*, int);
void    *emalloc(uint64_t);
void    *erealloc(void*, uint64_t);
void    error(Err);
void    error_c(Err, int);
void    error_s(Err, char*);
int execute(File*, Posn, Posn);
int filematch(File*, String*);
void    filename(File*);
File    *getfile(String*);
int getname(File*, String*, bool);
int64_t    getnum(void);
void    hiccough(char*);
void    inslist(List*, int, int64_t);
Address lineaddr(Posn, Address, int);
void    listfree(List*);
void    load(File*);
File    *lookfile(String*, int);
void    lookorigin(File*, Posn, Posn, int64_t);
int lookup(int);
void    move(File*, Address);
void    moveto(File*, Range);
File    *newfile(void);
void    nextmatch(File*, String*, Posn, int);
int newtmp(void);
void    notifyf(void*, char*);
void    panic(char*);
void    printposn(File*, int);
void    print_ss(char*, String*, String*);
void    print_s(char*, String*);
int rcv(void);
Range   rdata(List*, Posn, Posn);
Posn    readio(File*, bool*, bool);
void    rescue(void);
void    resetcmd(void);
void    resetsys(void);
void    resetxec(void);
void    rgrow(List*, Posn, Posn);
void    samerr(char*);
void    settempfile(void);
int skipbl(void);
void    snarf(File*, Posn, Posn, Buffer*, int);
void    sortname(File*);
void    startup(char*, int, char**, char**);
void    state(File*, int);
int statfd(int, uint64_t*, uint64_t*, int64_t*, int64_t*, int64_t*);
int statfile(char*, uint64_t*, uint64_t*, int64_t*, int64_t*, int64_t*);
void    Straddc(String*, int);
void    Strclose(String*);
int Strcmp(String*, String*);
void    Strdelete(String*, Posn, Posn);
void    Strdupl(String*, wchar_t*);
void    Strduplstr(String*, String*);
void    Strinit(String*);
void    Strinit0(String*);
void    Strinsert(String*, String*, Posn);
void    Strinsure(String*, uint64_t);
void    Strzero(String*);
int Strlen(wchar_t*);
char    *Strtoc(String*);
void    syserror(char*);
void    telldot(File*);
void    tellpat(void);
String  *tmpcstr(char*);
String  *tmprstr(wchar_t*, int);
void    freetmpstr(String*);
void    termcommand(void);
void    termwrite(char*);
File    *tofile(String*);
void    toterminal(File*, int);
void    trytoclose(File*);
void    trytoquit(void);
int undo(void);
void    update(void);
int waitfor(int);
void    warn(Warn);
void    warn_s(Warn, char*);
void    warn_SS(Warn, String*, String*);
void    warn_S(Warn, String*);
int whichmenu(File*);
void    writef(File*);
Posn    writeio(File*);

extern wchar_t samname[];  /* compiler dependent */
extern wchar_t *left[];
extern wchar_t *right[];

extern char *rsamname;  /* globals */
extern char *samterm;
extern char *sh;
extern char *shpath;
extern wchar_t genbuf[];
extern char *genc;
extern int  io;
extern bool  patset;
extern bool  quitok;
extern Address  addr;
extern Buffer   *undobuf;
extern Buffer   *snarfbuf;
extern Buffer   *plan9buf;
extern List file;
extern List tempfile;
extern File *cmd;
extern File *curfile;
extern File *lastfile;
extern Mod  modnum;
extern Posn cmdpt;
extern Posn cmdptadv;
extern Rangeset sel;
extern String   cmdstr;
extern String   genstr;
extern String   lastpat;
extern String   lastregexp;
extern String   plan9cmd;
extern bool  downloaded;
extern bool  eof;
extern bool  bpipeok;
extern bool  panicking;
extern wchar_t empty[];
extern int  termlocked;
extern bool  noflush;

#include "mesg.h"

void    outTs(Hmesg, int);
void    outT0(Hmesg);
void    outTl(Hmesg, int64_t);
void    outTslS(Hmesg, int, int64_t, String*);
void    outTS(Hmesg, String*);
void    outTsS(Hmesg, int, String*);
void    outTsllS(Hmesg, int, int64_t, int64_t, String*);
void    outTsll(Hmesg, int, int64_t, int64_t);
void    outTsl(Hmesg, int, int64_t);
void    outTsv(Hmesg, int, int64_t);
void    outstart(Hmesg);
void    outcopy(int, void*);
void    outshort(int);
void    outlong(int64_t);
void    outsend(void);
void    outflush(void);
