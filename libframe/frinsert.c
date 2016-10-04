/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>

#define DELTA   25
#define TMPSIZE 256
static Frame        frame;

static
Point
bxscan(Frame *f, wchar_t *sp, wchar_t *ep, Point *ppt)
{
    int w, c, nb, delta, nl, nr, rw;
    Frbox *b;
    char *s, tmp[TMPSIZE+3];    /* +3 for rune overflow */
    uint8_t *p;

    frame.r = f->r;
    frame.b = f->b;
    frame.font = f->font;
    frame.fheight = f->font->ascent + f->font->descent;
    frame.maxtab = f->maxtab;
    frame.left = f->left;
    frame.nbox = 0;
    frame.nchars = 0;
    delta = DELTA;
    nl = 0;
    for(nb=0; sp<ep && nl<=f->maxlines; nb++,frame.nbox++){
        if(nb == frame.nalloc){
            _frgrowbox(&frame, delta);
            if(delta < 10000)
                delta *= 2;
        }
        b = &frame.box[nb];
        c = *sp;
        if(c=='\t' || c=='\n'){
            b->a.b.bc = c;
            b->wid = 5000;
            b->a.b.minwid = (c=='\n')? 0 : charwidth(frame.font, ' ');
            b->nrune = -1;
            if(c=='\n')
                nl++;
            frame.nchars++;
            sp++;
        }else{
            s = tmp;
            nr = 0;
            w = 0;
            while(sp < ep){
                c = *sp;
                if(c=='\t' || c=='\n')
                    break;
                rw = runetochar(s, *sp);
                if(s+rw >= tmp+TMPSIZE)
                    break;
                w += charwidth(frame.font, c);
                sp++;
                s += rw;
                nr++;
            }
            *s++ = 0;
            p = _frallocstr(s-tmp);
            b = &frame.box[nb];
            b->a.ptr = p;
            memmove(p, tmp, s-tmp);
            b->wid = w;
            b->nrune = nr;
            frame.nchars += nr;
        }
    }
    _frcklinewrap0(f, ppt, &frame.box[0]);
    return _frdraw(&frame, *ppt);
}

static
void
chopframe(Frame *f, Point pt, uint64_t p, int bn)
{
    Frbox *b;

    for(b = &f->box[bn]; ; b++){
        if(b >= &f->box[f->nbox])
            berror("endofframe");
        _frcklinewrap(f, &pt, b);
        if(pt.y >= f->r.max.y)
            break;
        p += NRUNE(b);
        _fradvance(f, &pt, b);
    }
    f->nchars = p;
    f->nlines = f->maxlines;
    if(b<&f->box[f->nbox])              /* BUG */
        _frdelbox(f, (int)(b-f->box), f->nbox-1);
}

void
frinsert(Frame *f, wchar_t *sp, wchar_t *ep, uint64_t p0)
{
    Point pt0, pt1, ppt0, ppt1, pt;
    Frbox *b;
    int n, n0, nn0, y;
    Rectangle r;
    static struct{
        Point pt0, pt1;
    }*pts;
    static int nalloc=0;
    int npts;

    if(p0>f->nchars || sp==ep || f->b==0)
        return;
    n0 = _frfindbox(f, 0, 0, p0);
    nn0 = n0;
    pt0 = _frptofcharnb(f, p0, n0);
    ppt0 = pt0;
    pt1 = bxscan(f, sp, ep, &ppt0);
    ppt1 = pt1;
    if(n0 < f->nbox){
        _frcklinewrap(f, &pt0, b = &f->box[n0]);    /* for frselectf() */
        _frcklinewrap0(f, &ppt1, b);
    }
    f->modified = true;
    /*
     * ppt0 and ppt1 are start and end of insertion as they will appear when
     * insertion is complete. pt0 is current location of insertion position
     * (p0); pt1 is terminal point (without line wrap) of insertion.
     */
    if(p0==f->p0 && p0==f->p1)      /* quite likely */
        frselectf(f, pt0, pt0, F&~D);
    else
        frselectp(f, F&~D);
    /*
     * Find point where old and new x's line up
     * Invariants:
     *  pt0 is where the next box (b, n0) is now
     *  pt1 is where it will be after then insertion
     * If pt1 goes off the rectangle, we can toss everything from there on
     */
    for(b = &f->box[n0],npts=0;
         pt1.x!=pt0.x && pt1.y!=f->r.max.y && n0<f->nbox; b++,n0++,npts++){
        _frcklinewrap(f, &pt0, b);
        _frcklinewrap0(f, &pt1, b);
        if(b->nrune > 0){
            n = _frcanfit(f, pt1, b);
            if(n == 0)
                berror("_frcanfit==0");
            if(n != b->nrune){
                _frsplitbox(f, n0, n);
                b = &f->box[n0];
            }
        }
        if(npts == nalloc){
            pts = realloc(pts, (npts+DELTA)*sizeof(pts[0]));
            nalloc += DELTA;
            b = &f->box[n0];
        }
        pts[npts].pt0 = pt0;
        pts[npts].pt1 = pt1;
        /* has a text box overflowed off the frame? */
        if(pt1.y == f->r.max.y)
            break;
        _fradvance(f, &pt0, b);
        pt1.x += _frnewwid(f, pt1, b);
    }
    if(pt1.y > f->r.max.y)
        berror("frinsert pt1 too far");
    if(pt1.y==f->r.max.y && n0<f->nbox){
        f->nchars -= _frstrlen(f, n0);
        _frdelbox(f, n0, f->nbox-1);
    }
    if(n0 == f->nbox)
        f->nlines = (pt1.y-f->r.min.y)/f->fheight+(pt1.x>f->left);
    else if(pt1.y!=pt0.y){
        int q0, q1;

        y = f->r.max.y;
        q0 = pt0.y+f->fheight;
        q1 = pt1.y+f->fheight;
        f->nlines += (q1-q0)/f->fheight;
        if(f->nlines > f->maxlines)
            chopframe(f, ppt1, p0, nn0);
        if(pt1.y < y){
            r = f->r;
            r.min.y = q0;
            r.max.y = y-(q1-q0);
            if(q1 < y)
                bitblt2(f->b, Pt(f->r.min.x, q1), f->b, r, S, 0, f->bg);
            r.min = pt0;
            r.max.y = q0;
            bitblt2(f->b, pt1, f->b, r, S, 0, f->bg);
        }
    }
    /*
     * Move the old stuff down to make room.  The loop will move the stuff
     * between the insertion and the point where the x's lined up.
     * The bitblt2 above moved everything down after the point they lined up.
     */
    for((y=pt1.y==f->r.max.y?pt1.y:0),b = &f->box[n0-1]; --npts>=0; --b){
        pt = pts[npts].pt1;
        if(b->nrune > 0){
            r.min = pts[npts].pt0;
            r.max = r.min;
            r.max.x += b->wid;
            r.max.y += f->fheight;
            bitblt2(f->b, pt, f->b, r, S, 0, f->bg);
            if(pt.y < y){   /* clear bit hanging off right */
                r.min = pt;
                r.max = pt;
                r.min.x += b->wid;
                r.max.x = f->r.max.x;
                r.max.y += f->fheight;
                bitblt2(f->b, r.min, f->b, r, 0, 0, f->bg);
            }
            y = pt.y;
        }else{
            r.min = pt;
            r.max = pt;
            r.max.x += b->wid;
            r.max.y += f->fheight;
            if(r.max.x >= f->r.max.x)
                r.max.x = f->r.max.x;
            bitblt2(f->b, r.min, f->b, r, 0, 0, f->bg);
            y = (pt.x == f->left)? pt.y : 0;
        }
    }
    frselectf(f, ppt0, ppt1, 0);
    _frredraw(&frame, ppt0);
    _fraddbox(f, nn0, frame.nbox);
    for(n=0; n<frame.nbox; n++)
        f->box[nn0+n] = frame.box[n];
    if(nn0>0 && f->box[nn0-1].nrune>=0 && ppt0.x-f->box[nn0-1].wid>=(int)f->left){
        --nn0;
        ppt0.x -= f->box[nn0].wid;
    }
    n0 += frame.nbox;
    _frclean(f, ppt0, nn0, n0<f->nbox-1? n0+1 : n0);
    f->nchars += frame.nchars;
    if(f->p0 >= p0)
        f->p0 += frame.nchars;
    if(f->p0 > f->nchars)
        f->p0 = f->nchars;
    if(f->p1 >= p0)
        f->p1 += frame.nchars;
    if(f->p1 > f->nchars)
        f->p1 = f->nchars;
    frselectp(f, F&~D);
}
