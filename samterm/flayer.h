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
    long        origin; /* offset of first char in flayer */
    long        p0, p1;
    long        click;  /* time at which selection click occurred, in HZ */
    Rune        *(*textfn)(Flayer*, long, uint64_t*);
    int     user0;
    void        *user1;
    Rectangle   entire;
    Rectangle   scroll;
    Vis     visible;
};

void    flborder(Flayer*, int);
void    flclose(Flayer*);
void    fldelete(Flayer*, long, long);
void    flfp0p1(Flayer*, uint64_t*, uint64_t*);
void    flinit(Flayer*, Rectangle, XftFont*, uint64_t bg);
void    flinsert(Flayer*, Rune*, Rune*, long);
void    flnew(Flayer*, Rune *(*fn)(Flayer*, long, uint64_t*), int, void*);
int flprepare(Flayer*);
Rectangle flrect(Flayer*, Rectangle);
void    flrefresh(Flayer*, Rectangle, int);
void    flreshape(Rectangle);
int flselect(Flayer*);
void    flsetselect(Flayer*, long, long);
void    flstart(Rectangle);
void    flupfront(Flayer*);
Flayer  *flwhich(Point);

#define FLMARGIN    4
#define FLSCROLLWID 12
#define FLGAP       4
