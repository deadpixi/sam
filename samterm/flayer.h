/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#undef None
typedef enum Vis{
    None=0,
    Some,
    All
}Vis;

enum{
    Clicktime=1000      /* one second */
};

typedef struct Flayer Flayer;

/* note that we track background color, but not foreground
 * all layers have the same foreground color
 */
struct Flayer
{
    uint64_t bg;
    Frame       f;
    int64_t        origin; /* offset of first char in flayer */
    int64_t        p0, p1;
    int64_t        click;  /* time at which selection click occurred, in HZ */
    wchar_t        *(*textfn)(Flayer*, int64_t, uint64_t*);
    int     user0;
    void        *user1;
    Rectangle   entire;
    Rectangle   scroll;
    Vis     visible;
};

void    flborder(Flayer*, bool);
void    flclose(Flayer*);
void    fldelete(Flayer*, int64_t, int64_t);
void    flfp0p1(Flayer*, uint64_t*, uint64_t*);
void    flinit(Flayer*, Rectangle, XftFont*, uint64_t bg);
void    flinsert(Flayer*, wchar_t*, wchar_t*, int64_t);
void    flnew(Flayer*, wchar_t *(*fn)(Flayer*, int64_t, uint64_t*), int, void*);
int flprepare(Flayer*);
Rectangle flrect(Flayer*, Rectangle);
void    flrefresh(Flayer*, Rectangle, int);
void    flreshape(Rectangle);
bool flselect(Flayer*);
void    flsetselect(Flayer*, int64_t, int64_t);
void    flstart(Rectangle);
void    flupfront(Flayer*);
Flayer  *flwhich(Point);

#define FLMARGIN    4
#define FLSCROLLWID 12
#define FLGAP       4
