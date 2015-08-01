/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <libc.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <libg.h>
#include <stdio.h>

void cont(char *);
void putstring(char *);
void colorinit(void);
void printcolmap(void);
void invertcolmap(void);

unsigned char arrowset[] =
	{0x00, 0x00, 0x7F, 0xC0, 0x7F, 0x00, 0x7C, 0x00,
	 0x7E, 0x00, 0x7F, 0x00, 0x6F, 0x80, 0x67, 0xC0,
	 0x43, 0xE0, 0x41, 0xF0, 0x00, 0xF8, 0x00, 0x7C,
	 0x00, 0x3E, 0x00, 0x1C, 0x00, 0x08, 0x00, 0x00};

char *colors[] = { "Black", "Red", "Green", "Yellow",
		"Cyan", "Magenta", "Blue", "White" };
RGB colordefs[] = {
	{ 0,0,0 },					/* black */
	{0xFFFFFFFF,	0x00000000,	0x00000000},	/* red */
	{0x00000000,	0xFFFFFFFF,	0x00000000},	/* green */
	{0xFFFFFFFF,	0xFFFFFFFF,	0x00000000},	/* yellow */
	{0x00000000,	0xFFFFFFFF,	0xFFFFFFFF},	/* cyan */
	{0xFFFFFFFF,	0x00000000,	0xFFFFFFFF},	/* magenta */
	{0x00000000,	0x00000000,	0xFFFFFFFF},	/* blue */
	{0xFFFFFFFF,	0xFFFFFFFF,	0xFFFFFFFF},	/* white */
};
#define Ncol (sizeof(colordefs)/sizeof(colordefs[0]))
unsigned long rgbval[Ncol];
Bitmap *rgbbitmap[Ncol];

main(int argc, char **argv)
{
	Point p1,p2,p3;
	Mouse m;
	int r,rx,ry;
	int n, i;
	char *m3gen(int);
	static Menu menu3 = { (char **) 0, m3gen, 0 };
	char *p, buf[200];
	Bitmap *bm, *bm2;
	RGB cmap[256];

	xtbinit(0,0,&argc,argv,0);
	einit(Ekeyboard|Emouse);
	p1 = add(screen.r.min, Pt(15,15));
	p2 = sub(screen.r.max, Pt(15,15));
	p3 = divpt(add(p1,p2),2);
	fprintf(stderr, "segment(&screen, (%d,%d), (%d,%d), ~0, S)\n",
		p1.x,p1.y,p2.x,p2.y);
	segment(&screen, p1, p2, ~0, S);
	cont("point");
	fprintf(stderr, "point(&screen, (%d,%d), ~0, S)\n", p1.x,p1.y);
	point(&screen, p1, ~0, S);
	cont("circle");
	rx = p3.x - p1.x;
	ry = p3.y - p1.y;
	r = (rx < ry)? rx : ry;
	fprintf(stderr, "circle(&screen, (%d,%d), %d, ~0, S)\n",
		p3.x,p3.y,r);
	circle(&screen, p3, r, ~0, S);
	cont("disc");
	fprintf(stderr, "disc(&screen, (%d,%d), %d, ~0, S)\n",
		p3.x,p3.y,r);
	disc(&screen, p3, r, ~0, S);
	cont("clipped disc");
	fprintf(stderr, "clipr(&screen, ((%d,%d)(%d,%d))\n",
		p1.x+30, p1.y+5, p3.x-30, p3.y-5);
	clipr(&screen, Rect(p1.x+30, p1.y+5, p3.x-30, p3.y-5));
	fprintf(stderr, "disc(&screen, (%d,%d), %d, ~0, S)\n",
		p3.x,p3.y,r);
	disc(&screen, p3, r, ~0, S);
	clipr(&screen, screen.r);
	cont("ellipse");
	fprintf(stderr, "ellipse(&screen, (%d,%d), %d, %d, ~0, S)\n",
		p3.x,p3.y,r,r/2);
	ellipse(&screen, p3, r, r/2, ~0, S);
	cont("arc");
	fprintf(stderr, "arc(&screen, (%d,%d), (%d,%d), (%d,%d), ~0, S)\n",
		p3.x,p3.y, p3.x+r,p3.y, p3.x+r/2,p3.x-(int)(r*.866));
	arc(&screen, p3, Pt(p3.x+r,p3.y), Pt(p3.x+r/2,p3.x-(int)(r*.866)), ~0, S);
	if(screen.ldepth > 1){
		cont("color");
		colorinit();
		p3 = p1;
		rx *= 2;
		ry *= 2;
		for(i = 0; i<Ncol; i++) {
			texture(&screen, Rpt(p3,add(p3,Pt(rx,ry/Ncol))),
				rgbbitmap[i], S);
        		string(&screen, add(p3,Pt(15,15)), font, colors[i], DxorS);
			p3.y += ry/Ncol;
		}
		printcolmap();
		cont("invert colmap");
		invertcolmap();
		printcolmap();
		p3 = p1;
		for(i = 0; i<Ncol; i++) {
			texture(&screen, Rpt(p3,add(p3,Pt(rx,ry/Ncol))),
				rgbbitmap[i], S);
        		string(&screen, add(p3,Pt(15,15)), font, colors[i], DxorS);
			p3.y += ry/Ncol;
		}
		cont("restore colmap");
		invertcolmap();
	}
	cont("wrbitmap, border, and bitblt(S)");
	bm = balloc(Rect(0,0,16,16), 0);
	fprintf(stderr, "border (%d,%d,%d,%d), -2, F)\n",
		p1.x, p1.y, p1.x+16, p1.y+16);
	border(&screen, Rpt(p1, add(p1,Pt(16,16))), -2, F);
	wrbitmap(bm, 0, 16, arrowset);
	fprintf(stderr, "bitblt(&screen, (%d,%d), bm, (0,0,16,16), S)\n",
		p1.x,p1.y);
	bitblt(&screen, p1, bm, Rect(0,0,16,16), S);
	cont("mouse track (button 1)");
	do{
		m = emouse();
	} while(!(m.buttons&1));
	fprintf(stderr,"test tracking\n");
	while(m.buttons&1){
		point(&screen, m.xy, ~0, S);
		m = emouse();
	}
	cursorswitch(0);
	cont("menuhit (button 3)");
	do {
		do{
			m = emouse();
		} while(!(m.buttons&4));
		n = menuhit(3, &m, &menu3);
		fprintf(stderr, "button %d\n", n);
	} while (n != 0);
	cont("keyboard (end with \\n)");
	fprintf(stderr, "type something\n");
	for (p = buf; (*p = ekbd()) != '\n' && *p != '\r'; p++) {
		fprintf(stderr, "%c", *p);
		if (*p == '\b')
			p -= 2;
		if (p < buf-1)
			p = buf-1;
		p[1] = 0;
 		putstring(buf);
	}
	cont("done");
	exit(0);
}

void colorinit(void)	/* set up color definitions */
{
	int i;

	for (i = 0; i < Ncol; i++) {
		rgbval[i] = rgbpix(&screen, colordefs[i]);
		rgbbitmap[i] = balloc(Rect(0,0,1,1), screen.ldepth);
		point(rgbbitmap[i], Pt(0,0), rgbval[i], S);
	}
}

void printcolmap(void)
{
	int i, n;
	RGB cmap[256];

	rdcolmap(&screen, cmap);
	n = 1 << (1 << screen.ldepth);
	fprintf(stderr, "colormap, %d entries\n", n);
	for(i = 0; i < n; i++)
		fprintf(stderr, "%d:\t%.8x\t%.8x\t%.8x\n",
			i, cmap[i].red, cmap[i].green, cmap[i].blue);
}

void invertcolmap(void)
{
	int i, n;
	RGB cmap[256];

	rdcolmap(&screen, cmap);
	n = 1 << (1 << screen.ldepth);
	for(i = 0; i < n; i++) {
		cmap[i].red = ~cmap[i].red;
		cmap[i].green = ~cmap[i].green;
		cmap[i].blue = ~cmap[i].blue;
	}
	wrcolmap(&screen, cmap);
}

void
putstring(char *buf)
{
        Point p;
        static int jmax = 0, l;

	p = add(screen.r.min, Pt(20,20));
	bitblt(&screen, p, &screen, Rect(p.x, p.y, p.x+jmax, p.y+font->height), Zero);
        string(&screen, p, font, buf, F);
        if ((l = strwidth(font, buf)) > jmax)
                jmax = l;
}

void
cont(char *msg)
{
	Event ev;
	Point mp;

	while(event(&ev) != Ekeyboard)
		continue;
	bitblt(&screen, Pt(0,0), &screen, screen.r, Zero);
	mp = add(screen.r.min, Pt(20,20));
	string(&screen, mp, font, msg, S);
	while(event(&ev) != Ekeyboard)
		continue;
	bitblt(&screen, Pt(0,0), &screen, screen.r, Zero);
}

char *
m3gen(int n)
{
	static char *m3[] ={ "quit", "thing1", "thing2" };

	if (n < 0 || n > 2)
		return 0;
	else 
		return m3[n];
}

void
ereshaped(Rectangle r)
{
}
