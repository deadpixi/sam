/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */

typedef struct Frbox Frbox;
typedef struct Frame Frame;

struct Frbox
{
    int64_t        wid;        /* in pixels */
    int64_t        nrune;      /* <0 ==> negate and treat as break char */
    union{
        uint8_t   *ptr;
        struct{
            int16_t   bc; /* break char */
            int16_t   minwid;
        } b;
    } a;
};

/* note that we track background color, but not foreground
 * this is because the foreground color is the same for all frames
 */
struct Frame
{
    uint64_t bg;       /* background color */
    XftFont     *font;      /* of chars in the frame */
    Bitmap      *b;         /* on which frame appears */
    Rectangle   r;          /* in which text appears */
    Rectangle   entire;     /* of full frame */
    Frbox       *box;
    uint64_t       p0, p1;     /* selection */
    int16_t       left;       /* left edge of text */
    uint16_t      nbox, nalloc;
    uint16_t      maxtab;     /* max size of tab, in pixels */
    uint16_t      fheight;    /* font height, in pixels */
    uint16_t      nchars;     /* # runes in frame */
    uint16_t      nlines;     /* # lines with text */
    uint16_t      maxlines;   /* total # lines in frame */
    bool      lastlinefull;   /* last line fills frame */
    bool      modified;   /* changed since frselect() */
};

uint64_t   frcharofpt(Frame*, Point);
Point   frptofchar(Frame*, uint64_t);
int frdelete(Frame*, uint64_t, uint64_t);
void    frinsert(Frame*, wchar_t*, wchar_t*, uint64_t);
void    frselect(Frame*, Mouse*);
void    frselectp(Frame*, Fcode);
void    frselectf(Frame*, Point, Point, Fcode);
void    frinit(Frame*, Rectangle, XftFont*, Bitmap*, uint64_t);
void    frsetrects(Frame*, Rectangle, Bitmap*);
void    frclear(Frame*);
void    frgetmouse(void);

uint8_t   *_frallocstr(unsigned);
void    _frinsure(Frame*, int, unsigned);
Point   _frdraw(Frame*, Point);
void    _frgrowbox(Frame*, int);
void    _frfreebox(Frame*, int, int);
void    _frmergebox(Frame*, int);
void    _frdelbox(Frame*, int, int);
void    _frsplitbox(Frame*, int, int);
int _frfindbox(Frame*, int, uint64_t, uint64_t);
void    _frclosebox(Frame*, int, int);
int _frcanfit(Frame*, Point, Frbox*);
void    _frcklinewrap(Frame*, Point*, Frbox*);
void    _frcklinewrap0(Frame*, Point*, Frbox*);
void    _fradvance(Frame*, Point*, Frbox*);
int _frnewwid(Frame*, Point, Frbox*);
void    _frclean(Frame*, Point, int, int);
void    _frredraw(Frame*, Point);
void    _fraddbox(Frame*, int, int);
Point   _frptofcharptb(Frame*, uint64_t, Point, int);
Point   _frptofcharnb(Frame*, uint64_t, int);
int _frstrlen(Frame*, int);

extern int tabwidth;

#define NRUNE(b)    ((b)->nrune<0? 1 : (b)->nrune)
#define NBYTE(b)    strlen((char*)(b)->a.ptr)
