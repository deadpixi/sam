/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

/*
 * Use the id field in Cursor to hold the X id corresponding
 * to the cursor, so that it doesn't have to be recreated on
 * each cursorswitch.  This doesn't quite match the semantics
 * of Plan9 libg, since the user could create a cursor (say
 * with malloc) with garbage in the id field; or the user
 * could change the contents of the other fields and we
 * wouldn't know about it.  Neither of these happen in
 * existing uses of libg.
 */
static Cursor arrow =
{
	{-1, -1},
	{0xFF, 0xE0, 0xFF, 0xE0, 0xFF, 0xC0, 0xFF, 0x00,
	 0xFF, 0x00, 0xFF, 0x80, 0xFF, 0xC0, 0xFF, 0xE0,
	 0xE7, 0xF0, 0xE3, 0xF8, 0xC1, 0xFC, 0x00, 0xFE,
	 0x00, 0x7F, 0x00, 0x3E, 0x00, 0x1C, 0x00, 0x08,
	},
	{0x00, 0x00, 0x7F, 0xC0, 0x7F, 0x00, 0x7C, 0x00,
	 0x7E, 0x00, 0x7F, 0x00, 0x6F, 0x80, 0x67, 0xC0,
	 0x43, 0xE0, 0x41, 0xF0, 0x00, 0xF8, 0x00, 0x7C,
	 0x00, 0x3E, 0x00, 0x1C, 0x00, 0x08, 0x00, 0x00,
	}
};

static Bitmap *bsrc, *bmask;
static Rectangle crect = { 0, 0, 16, 16 };

void
cursorswitch(Cursor *c)
{
	if(c == 0)
		c = &arrow;
	if(c->id == 0){
		if(bsrc == 0){
			bsrc = balloc(crect, 0);
			bmask = balloc(crect, 0);
		}
		/*
		 * Cursor should have fg where "set" is 1,
		 * and bg where "clr" is 1 and "set" is 0,
		 * and should leave places alone where "set" and "clr" are both 0
		 */
		wrbitmap(bsrc, 0, 16, c->set);
#ifdef CURSORBUG
		/*
		 * Some X servers (e.g., Sun X-on-news for some color
		 * monitors) don't do XCreatePixmapCursor properly:
		 * only the mask gets displayed, all black
		 */
		wrbitmap(bmask, 0, 16, c->set);
#else
		wrbitmap(bmask, 0, 16, c->clr);
		bitblt(bmask, Pt(0,0), bsrc, crect, S|D);
#endif
		c->id = (int) XCreatePixmapCursor(_dpy, (Pixmap)bsrc->id, (Pixmap)bmask->id,
			&_fgcolor, &_bgcolor, -c->offset.x, -c->offset.y);
	}
	XDefineCursor(_dpy, (Window)screen.id, (xCursor)c->id);
}
