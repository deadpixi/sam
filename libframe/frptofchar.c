/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>

Point
_frptofcharptb(Frame *f, uint64_t p, Point pt, int bn)
{
    uint8_t *s;
    Frbox *b;
    int w, l;
    wchar_t r;

    for(b = &f->box[bn]; bn<f->nbox; bn++,b++){
        _frcklinewrap(f, &pt, b);
        if(p < (l=NRUNE(b))){
            if(b->nrune > 0)
                for(s=b->a.ptr; p>0; s+=w, p--){
                    w = chartorune(&r, (char*)s);
                    pt.x += charwidth(f->font, r);
                    if(r==0 || pt.x>f->r.max.x)
                        berror("frptofchar");
                }
            break;
        }
        p -= l;
        _fradvance(f, &pt, b);
    }
    return pt;
}

Point
frptofchar(Frame *f, uint64_t p)
{
    return _frptofcharptb(f, p, Pt(f->left, f->r.min.y), 0);
}

Point
_frptofcharnb(Frame *f, uint64_t p, int nb)    /* doesn't do final _fradvance to next line */
{
    Point pt;
    int nbox;

    nbox = f->nbox;
    f->nbox = nb;
    pt = _frptofcharptb(f, p, Pt(f->left, f->r.min.y), 0);
    f->nbox = nbox;
    return pt;
}

static
Point
_frgrid(Frame *f, Point p)
{
    p.y -= f->r.min.y;
    p.y -= p.y%f->fheight;
    p.y += f->r.min.y;
    if(p.x > f->r.max.x)
        p.x = f->r.max.x;
    return p;
}

uint64_t
frcharofpt(Frame *f, Point pt)
{
    Point qt;
    int w, bn, cstart;
    uint8_t *s;
    Frbox *b;
    uint64_t p;
    wchar_t r;

    pt = _frgrid(f, pt);
    qt.x = f->left;
    qt.y = f->r.min.y;
    for(b=f->box,bn=0,p=0; bn<f->nbox && qt.y<pt.y; bn++,b++){
        _frcklinewrap(f, &qt, b);
        if(qt.y >= pt.y)
            break;
        _fradvance(f, &qt, b);
        p += NRUNE(b);
    }
    for(; bn<f->nbox && qt.x<=pt.x; bn++,b++){
        _frcklinewrap(f, &qt, b);
        if(qt.y > pt.y)
            break;
        if(qt.x+b->wid > pt.x){
            if(b->nrune < 0)
                _fradvance(f, &qt, b);
            else{
                s = b->a.ptr;
                for(;;){
                    w = chartorune(&r, (char*)s);
                    if(r == 0)
                        berror("end of string in frcharofpt");
                    s += w;
                    cstart = qt.x;
                    qt.x += charwidth(f->font, r);
                    if(qt.x > pt.x){
                        if(qt.x - pt.x < pt.x - cstart)
                            p++;
                        break;
                    }
                    p++;
                }
            }
        }else{
            p += NRUNE(b);
            _fradvance(f, &qt, b);
        }
    }
    return p;
}
