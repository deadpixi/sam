/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>

#define SLOP    25

void
_fraddbox(Frame *f, int bn, int n)  /* add n boxes after bn, shift the rest up,
                 * box[bn+n]==box[bn] */
{
    int i;

    if(bn > f->nbox)
        berror("_fraddbox");
    if(f->nbox+n > f->nalloc)
        _frgrowbox(f, n+SLOP);
    for(i=f->nbox; --i>=bn; )
        f->box[i+n] = f->box[i];
    f->nbox+=n;
}

void
_frclosebox(Frame *f, int n0, int n1)   /* inclusive */
{
    int i;

    if(n0>=f->nbox || n1>=f->nbox || n1<n0)
        berror("_frclosebox");
    n1++;
    for(i=n1; i<f->nbox; i++)
        f->box[i-(n1-n0)] = f->box[i];
    f->nbox -= n1-n0;
}

void
_frdelbox(Frame *f, int n0, int n1) /* inclusive */
{
    if(n0>=f->nbox || n1>=f->nbox || n1<n0)
        berror("_frdelbox");
    _frfreebox(f, n0, n1);
    _frclosebox(f, n0, n1);
}

void
_frfreebox(Frame *f, int n0, int n1)    /* inclusive */
{
    int i;

    if(n1<n0)
        return;
    if(n0>=f->nbox || n1>=f->nbox)
        berror("_frfreebox");
    n1++;
    for(i=n0; i<n1; i++)
        if(f->box[i].nrune >= 0)
            free(f->box[i].a.ptr);
}

void
_frgrowbox(Frame *f, int delta)
{
    f->nalloc += delta;
    f->box = realloc(f->box, f->nalloc*sizeof(Frbox));
    if(f->box == 0)
        berror("_frgrowbox");
}

static
void
dupbox(Frame *f, int bn)
{
    uint8_t *p;

    if(f->box[bn].nrune < 0)
        berror("dupbox");
    _fraddbox(f, bn, 1);
    if(f->box[bn].nrune >= 0){
        p = _frallocstr(NBYTE(&f->box[bn])+1);
        strcpy((char*)p, (char*)f->box[bn].a.ptr);
        f->box[bn+1].a.ptr = p;
    }
}

static
uint8_t*
runeindex(uint8_t *p, int n)
{
    int i, w;
    wchar_t rune;

    for(i=0; i<n; i++,p+=w)
        w = chartorune(&rune, (char*)p);
    return p;
}

static
void
truncatebox(Frame *f, Frbox *b, int n)  /* drop last n chars; no allocation done */
{
    if(b->nrune<0 || b->nrune<n)
        berror("truncatebox");
    b->nrune -= n;
    runeindex(b->a.ptr, b->nrune)[0] = 0;
    b->wid = strwidth(f->font, (char *)b->a.ptr);
}

static
void
chopbox(Frame *f, Frbox *b, int n)  /* drop first n chars; no allocation done */
{
    if(b->nrune<0 || b->nrune<n)
        berror("chopbox");

    uint8_t *ri = runeindex(b->a.ptr, n);
    memmove(b->a.ptr, ri, strlen((char *)ri) + 1);
    b->nrune -= n;
    b->wid = strwidth(f->font, (char *)b->a.ptr);
}

void
_frsplitbox(Frame *f, int bn, int n)
{
    dupbox(f, bn);
    truncatebox(f, &f->box[bn], f->box[bn].nrune-n);
    chopbox(f, &f->box[bn+1], n);
}

void
_frmergebox(Frame *f, int bn)       /* merge bn and bn+1 */
{
    Frbox *b;

    b = &f->box[bn];
    _frinsure(f, bn, NBYTE(&b[0])+NBYTE(&b[1])+1);
    strcpy((char*)runeindex(b[0].a.ptr, b[0].nrune), (char*)b[1].a.ptr);
    b[0].wid += b[1].wid;
    b[0].nrune += b[1].nrune;
    _frdelbox(f, bn+1, bn+1);
}

int
_frfindbox(Frame *f, int bn, uint64_t p, uint64_t q)  /* find box containing q and put q on a box boundary */
{
    Frbox *b;

    for(b = &f->box[bn]; bn<f->nbox && p+NRUNE(b)<=q; bn++, b++)
        p += NRUNE(b);
    if(p != q)
        _frsplitbox(f, bn++, (int)(q-p));
    return bn;
}
