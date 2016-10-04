/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

/*
 * Libg applications are written assuming that black is ~0
 * and white is 0.  Some screens use the reverse convention.
 * We get the effect the application desired by seeing what
 * happens if both the source and dest are converted to the
 * black==~0 convention, and then converting the dest back
 * to whatever convention it uses.
 *
 * Offscreen bitmaps of depth 1 use the black==~0 convention.
 *
 * Bitmaps of depth > 1 are probably in color.  Libg operations that
 * would produce a 1 should produce the foreground color, and
 * libg operations that would produce a 0 should produce the background
 * color.  Operations that use bitmaps of depth > 1 as source
 * should interpret the foreground pixel as "black" (1) and the
 * background pixel as "white" (0).  It is hard to make this work,
 * but the important cases are Fcodes Zero, F, S, and S^D, so
 * we make sure that those work.  When a fill value is given for
 * a bitmap of depth > 1, assume ~0 means foreground, but otherwise
 * take any other value literally (assume it came from rgbpix).
 * This may be wrong for the case of 0, but libg programmers
 * usually use Fcode Zero instead of passing 0 with Fcode S.
 *
 * We assume there are at most two depths of bitmaps: depth 1
 * and depth of the screen.
 */

/*
 * gx func code corresponding to libg func code when both
 * source and dest use 1 for black.  This is a straight translation.
 */
static int gx[16] = {
    GXclear,        /* Zero */
    GXnor,          /* DnorS */
    GXandInverted,      /* DandnotS */
    GXcopyInverted,     /* notS */
    GXandReverse,       /* notDandS */
    GXinvert,       /* notD */
    GXxor,          /* DxorS */
    GXnand,         /* DnandS */
    GXand,          /* DandS */
    GXequiv,        /* DxnorS */
    GXnoop,         /* D */
    GXorInverted,       /* DornotS */
    GXcopy,         /* S */
    GXorReverse,        /* notDorS */
    GXor,           /* DorS */
    GXset,          /* F */
};

/*
 * gx func code corresponding to libg func code when 0 means black
 * in dst and 1 means black in src. These means the table has op'
 * where dst <- dst op' src == not ( not(dst)  op  src ).
 * The comment on each line is op, in Fcode terms.
 */
static int d0s1gx[16] = {
    GXset,          /* Zero */
    GXorReverse,        /* DnorS */
    GXor,           /* DandnotS */
    GXcopy,         /* notS */
    GXnand,         /* notDandS */
    GXinvert,       /* notD */
    GXxor,          /* DxorS */
    GXandReverse,       /* DnandS */
    GXorInverted,       /* DandS */
    GXequiv,        /* DxnorS */
    GXnoop,         /* D */
    GXand,          /* DornotS */
    GXcopyInverted,     /* S */
    GXnor,          /* notDorS */
    GXandInverted,      /* DorS */
    GXclear,        /* F */
};
/*
 * gx func code corresponding to libg func code when 1 means black
 * in dst and 0 means black in src. These means the table has op'
 * where dst <- dst op' src == dst  op  not(src) )
 * The comment on each line is op, in Fcode terms.
 */
static int d1s0gx[16] = {
    GXclear,        /* Zero */
    GXandReverse,       /* DnorS */
    GXand,          /* DandnotS */
    GXcopy,         /* notS */
    GXnor,          /* notDandS */
    GXinvert,       /* notD */
    GXequiv,        /* DxorS */
    GXorReverse,        /* DnandS */
    GXandInverted,      /* DandS */
    GXxor,          /* DxnorS */
    GXnoop,         /* D */
    GXor,           /* DornotS */
    GXcopyInverted,     /* S */
    GXnand,         /* notDorS */
    GXorInverted,       /* DorS */
    GXset,          /* F */
};

/*
 * gx func code corresponding to libg func code when 0 means black
 * in both the src and the dst. These means the table has op'
 * where dst <- dst op' src == not (not(dst)  op  not(src)) )
 * The comment on each line is op, in Fcode terms.
 */
static int d0s0gx[16] = {
    GXset,          /* Zero */
    GXnand,         /* DnorS */
    GXorInverted,       /* DandnotS */
    GXcopyInverted,     /* notS */
    GXorReverse,        /* notDandS */
    GXinvert,       /* notD */
    GXequiv,        /* DxorS */
    GXnor,          /* DnandS */
    GXor,           /* DandS */
    GXxor,          /* DxnorS */
    GXnoop,         /* D */
    GXandInverted,      /* DornotS */
    GXcopy,         /* S */
    GXandReverse,       /* notDorS */
    GXand,          /* DorS */
    GXclear,        /* F */
};

/*
 * 1 for those Fcodes that are degenerate (don't involve src)
 */
static int degengc[16] = {
    1,          /* Zero */
    0,          /* DnorS */
    0,          /* DandnotS */
    0,          /* notS */
    0,          /* notDandS */
    1,          /* notD */
    0,          /* DxorS */
    0,          /* DnandS */
    0,          /* DandS */
    0,          /* DxnorS */
    1,          /* D */
    0,          /* DornotS */
    0,          /* S */
    0,          /* notDorS */
    0,          /* DorS */
    1,          /* F */
};

/*
 * GCs are all for same screen, and depth is either 1 or screen depth.
 * Return a GC for the depth of b, with values as specified by gcv.
 *
 * Also, set (or unset) the clip rectangle if necessary.
 * (This implementation should be improved if setting a clip rectangle is not rare).
 */
GC
_getgc(Bitmap *b, uint64_t gcvm, XGCValues *pgcv)
{
    static GC gc0, gcn;
    static bool clipset = false;
    GC g;
    XRectangle xr;

    g = (b->ldepth==0)? gc0 : gcn;
    if(!g){
        g = XCreateGC(_dpy, (Drawable)b->id, gcvm, pgcv);
        if(b->ldepth==0)
            gc0 = g;
        else
            gcn = g;
    } else
        XChangeGC(_dpy, g, gcvm, pgcv);
    if(b->flag&CLIP){
        xr.x = b->clipr.min.x;
        xr.y = b->clipr.min.y;
        xr.width = Dx(b->clipr);
        xr.height = Dy(b->clipr);
        if(b->flag&SHIFT){
            xr.x -= b->r.min.x;
            xr.y -= b->r.min.y;
        }
        XSetClipRectangles(_dpy, g, 0, 0, &xr, 1, YXBanded);
        clipset = true;
    }else if(clipset){
        pgcv->clip_mask = None;
        XChangeGC(_dpy, g, GCClipMask, pgcv);
        clipset = false;
    }
    return g;
}

/*
 * Return a GC that will fill bitmap b using a pixel value v and Fcode f.
 * Pixel value v is according to libg convention, so 0 means
 * white (or background) and ~0 means black (or foreground).
 */
GC
_getfillgc(Fcode f, Bitmap *b, uint64_t val)
{
    return _getfillgc2(f, b, val, _fgpixel, _bgpixel);
}

GC
_getfillgc2(Fcode f, Bitmap *b, uint64_t val, uint64_t fg, uint64_t bg)
{
    int xf, m;
    uint64_t v, spix, vmax;
    XGCValues gcv;

    f &= F;
    vmax = _ld2dmask[b->ldepth];
    v = val & vmax;
    spix = v;
    xf = GXcopy;
    m = b->flag;
    if(m & DP1){
        xf = (m&BL1)? gx[f] : d0s1gx[f];
    }else{
        switch(f){
        case Zero:
        labZero:
            spix = bg;
            break;
        case F:
        labF:
            spix = fg;
            break;
        case D:
        labD:
            xf = GXnoop;
            break;
        case notD:
        labnotD:
            xf = GXxor;
            spix = fg^bg;
            break;
        case S:
            if(val == ~0)
                spix = fg;
            else
                spix = v;
            break;
        case notS:
            if(val == ~0)
                spix = bg;
            else
                spix = v;
            break;
        case DxorS:
            xf = GXxor;
            if(val == ~0)
                spix = fg^bg;
            else
                spix = v;
            break;
        case DxnorS:
            xf = GXxor;
            if(val == 0)
                spix = fg^bg;
            else
                spix = v;
            break;
        default:
            /* hard to do anything other than v==0 or v==~0 case */
            if(v < vmax-v){
                /* v is closer to 0 than vmax */
                switch(f&~S){
                case D&~S:  goto labD;
                case notD&~S:   goto labnotD;
                case Zero&~S:   goto labZero;
                case F&~S:  goto labF;
                }
            }else{
                /* v is closer to vmax than 0 */
                switch(f&S){
                case D&S:   goto labD;
                case notD&S:    goto labnotD;
                case Zero&S:    goto labZero;
                case F&S:   goto labF;
                }
            }
            
        }
    }
    gcv.foreground = spix;
    gcv.function = xf;
    return _getgc(b, GCForeground|GCFunction, &gcv);
}

/*
 * Return a GC to be used to copy an area from bitmap sb to
 * bitmap db.  Sometimes the calling function shouldn't use
 * XCopyArea, but instead should use XCopyPlane or XFillRectangle.
 * The *bltfunc arg is set to one of UseCopyArea, UseCopyPlane,
 * UseFillRectangle.
 */
GC
_getcopygc(Fcode f, Bitmap *db, Bitmap *sb, int *bltfunc)
{
    return _getcopygc2(f, db, sb, bltfunc, _fgpixel, _bgpixel);
}

GC
_getcopygc2(Fcode f, Bitmap *db, Bitmap *sb, int *bltfunc, uint64_t fg, uint64_t bg)
{
    uint64_t spix, df, sf;
    int xf, c;
    XGCValues gcv;
    uint64_t gcvm;

    spix = xf = 0;
    f &= F;
    gcvm = 0;
    df = db->flag;
    if(degengc[f]){
        *bltfunc = UseFillRectangle;
        if(df&SCR || !(df&DP1)){
            // nothing XXX
        }else{
            /* must be DP1 and BL1 */
            fg = 1;
            bg = 0;
        }
        switch(f){
        case Zero:
            xf = GXcopy;
            spix = bg;
            break;
        case F:
            xf = GXcopy;
            spix = fg;
            break;
        case D:
            xf = GXnoop;
            spix = fg;
            break;
        case notD:
            xf = GXxor;
            spix = fg^bg;
            break;
        default:
            /* ignored */
            break;
        }
        gcv.function = xf;
        gcv.foreground = spix;
        gcvm = GCFunction|GCForeground;
    }else{
        /* src is involved in f */

#define code(f1,f2) ((((f1)&(DP1|BL1))<<2)|((f2)&(DP1|BL1)))

        sf = sb->flag;
        c = code(df,sf);
        *bltfunc = UseCopyArea;
        switch(code(df,sf)){
        case code(DP1|BL1,DP1|BL1):
        case code(BL1,BL1):
            xf = gx[f];
            break;
        case code(DP1|BL1,DP1):
            xf = d1s0gx[f];
            break;
        case code(DP1,DP1|BL1):
            xf = d0s1gx[f];
            break;
        case code(DP1,DP1):
        case code(0,0):
            xf = d0s0gx[f];
            break;
        default:
            /*
             * One bitmap has depth 1, the other has screen depth.
             * We know the bitmap must have BL1.
             * CopyPlane must be used; it won't really work
             * for more than fcode==S.
             */

            *bltfunc = UseCopyPlane;
            xf = GXcopy;
            switch(c){

            case code(0,DP1|BL1):
            case code(BL1,DP1|BL1):
                // nothing XXX
                break;
            case code(DP1|BL1,0):
                fg = 0;
                bg = 1;
                break;
            case code(DP1|BL1,BL1):
                fg = 1;
                bg = 0;
                break;
            default:
                berror("bad combination of copy bitmaps");
            }
            gcv.foreground = fg;
            gcv.background = bg;
            gcvm |= GCForeground|GCBackground;
        }
        gcv.function = xf;
        gcvm |= GCFunction;
    
#undef code
    }

    return _getgc(db, gcvm, &gcv);
}
