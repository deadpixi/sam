/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

void
_ldconvert(char *in, int inld, char *out, int outld, int w, int h)
{
    int a, b, i, j, i1, j1, j2, mask;
    int ind, inl, outd, outl;
    int hh, ww;
    char    *p, *q;

    i1 = 8 >> inld;
    j1 = 8 >> outld;
    ind = 1 << inld;
    outd = 1 << outld;
    inl = ((w << inld) + 7)/8;
    outl = ((w << outld) + 7)/8;
    b = 0;

    if (ind > outd) {
        mask = 256 - (256 >> outd);
        for (hh = 0; hh < h; hh++, in += inl, out += outl)
            for (p = in, q = out, ww = 0; ww < w; ww++) {
                for (j = j1; j > 0; ) {
                    a = *p++;
                    for (i = i1; i > 0; i--, j--) {
                        b |= a & mask;
                        a <<= ind;
                        b <<= outd;
                    }
                }
                *q++ = (b >> 8);
            }
    } else {
        j2 = 1 << (outld - inld);
        mask = 256 - (256 >> ind);
        for (hh = 0; hh < h; hh++, in += inl, out += outl)
            for (p = in, q = out, ww = 0; ww < w; ww++) {
                a = *p++;
                for (i = i1; i > 0; ) {
                    for (j = j1; j > 0; j--, i--) {
                        b |= a & mask;
                        a <<= ind;
                        b <<= outd;
                    }
                    for (j = j2; j > 0; j--)
                        b |= (b << ind);
                    *q++ = (b >> 8);
                }
            }
    }
}
