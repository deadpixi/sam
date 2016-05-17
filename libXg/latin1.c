/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
/* Changes copyright 2014-2014 Rob King. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPPING_MAX 65535

struct latin
{
	unsigned short	l;
	unsigned char	c[2];
};

struct latin latintab[] = {
	0x00a1,	'!','!',	/* spanish initial ! */
	0x00a2,	'c','$',	/* cent */
	0x00a3,	'l','$',	/* pound sterling */
	0x00a4,	'g','$',	/* general currency */
	0x00a5,	'y','$',	/* yen */
	0x00a6,	'|','|',	/* broken vertical bar */
	0x00a7,	'S','S',	/* section symbol */
	0x00a8,	'\"','\"',	/* dieresis */
	0x00a9,	'c','O',	/* copyright */
	0x00aa,	's','a',	/* super a, feminine ordinal */
	0x00ab,	'<','<',	/* left angle quotation */
	0x00ac,	'n','o',	/* not sign, hooked overbar */
	0x00ad,	'-','-',	/* soft hyphen */
	0x00ae,	'r','O',	/* registered trademark */
	0x00af,	'_','_',	/* macron */
	0x00b0,	'd','e',	/* degree */
	0x00b1,	'+','-',	/* plus-minus */
	0x00b2,	's','2',	/* sup 2 */
	0x00b3,	's','3',	/* sup 3 */
	0x00b4,	'\'','\'',	/* acute accent */
	0x00b5,	'm','i',	/* micron */
	0x00b6,	'p','g',	/* paragraph (pilcrow) */
	0x00b7,	'.','.',	/* centered . */
	0x00b8,	',',',',	/* cedilla */
	0x00b9,	's','1',	/* sup 1 */
	0x00ba,	's','o',	/* super o, masculine ordinal */
	0x00bb,	'>','>',	/* right angle quotation */
	0x00bc,	'1','4',	/* 1/4 */
	0x00bd,	'1','2',	/* 1/2 */
	0x00be,	'3','4',	/* 3/4 */
	0x00bf,	'?','?',	/* spanish initial ? */
	0x00c0,	'`','A',	/* A grave */
	0x00c1,	'\'','A',	/* A acute */
	0x00c2,	'^','A',	/* A circumflex */
	0x00c3,	'~','A',	/* A tilde */
	0x00c4,	'\"','A',	/* A dieresis */
	0x00c5,	'o','A',	/* A circle */
	0x00c6,	'A','E',	/* AE ligature */
	0x00c7,	',','C',	/* C cedilla */
	0x00c8,	'`','E',	/* E grave */
	0x00c9,	'\'','E',	/* E acute */
	0x00ca,	'^','E',	/* E circumflex */
	0x00cb,	'\"','E',	/* E dieresis */
	0x00cc,	'`','I',	/* I grave */
	0x00cd,	'\'','I',	/* I acute */
	0x00ce,	'^','I',	/* I circumflex */
	0x00cf,	'\"','I',	/* I dieresis */
	0x00d0,	'D','-',	/* Eth */
	0x00d1,	'~','N',	/* N tilde */
	0x00d2,	'`','O',	/* O grave */
	0x00d3,	'\'','O',	/* O acute */
	0x00d4,	'^','O',	/* O circumflex */
	0x00d5,	'~','O',	/* O tilde */
	0x00d6,	'\"','O',	/* O dieresis */
	0x00d7,	'm','u',	/* times sign */
	0x00d8,	'/','O',	/* O slash */
	0x00d9,	'`','U',	/* U grave */
	0x00da,	'\'','U',	/* U acute */
	0x00db,	'^','U',	/* U circumflex */
	0x00dc,	'\"','U',	/* U dieresis */
	0x00dd,	'\'','Y',	/* Y acute */
	0x00de,	'|','P',	/* Thorn */
	0x00df,	's','s',	/* sharp s */
	0x00e0,	'`','a',	/* a grave */
	0x00e1,	'\'','a',	/* a acute */
	0x00e2,	'^','a',	/* a circumflex */
	0x00e3,	'~','a',	/* a tilde */
	0x00e4,	'\"','a',	/* a dieresis */
	0x00e5,	'o','a',	/* a circle */
	0x00e6,	'a','e',	/* ae ligature */
	0x00e7,	',','c',	/* c cedilla */
	0x00e8,	'`','e',	/* e grave */
	0x00e9,	'\'','e',	/* e acute */
	0x00ea,	'^','e',	/* e circumflex */
	0x00eb,	'\"','e',	/* e dieresis */
	0x00ec,	'`','i',	/* i grave */
	0x00ed,	'\'','i',	/* i acute */
	0x00ee,	'^','i',	/* i circumflex */
	0x00ef,	'\"','i',	/* i dieresis */
	0x00f0,	'd','-',	/* eth */
	0x00f1,	'~','n',	/* n tilde */
	0x00f2,	'`','o',	/* o grave */
	0x00f3,	'\'','o',	/* o acute */
	0x00f4,	'^','o',	/* o circumflex */
	0x00f5,	'~','o',	/* o tilde */
	0x00f6,	'\"','o',	/* o dieresis */
	0x00f7,	'-',':',	/* divide sign */
	0x00f8,	'/','o',	/* o slash */
	0x00f9,	'`','u',	/* u grave */
	0x00fa,	'\'','u',	/* u acute */
	0x00fb,	'^','u',	/* u circumflex */
	0x00fc,	'\"','u',	/* u dieresis */
	0x00fd,	'\'','y',	/* y acute */
	0x00fe,	'|','p',	/* thorn */
	0x00ff,	'\"','y',	/* y dieresis */
	0x2654,	'w','k',	/* chess white king */
	0x2655,	'w','q',	/* chess white queen */
	0x2656,	'w','r',	/* chess white rook */
	0x2657,	'w','b',	/* chess white bishop */
	0x2658,	'w','n',	/* chess white knight */
	0x2659,	'w','p',	/* chess white pawn */
	0x265a,	'b','k',	/* chess black king */
	0x265b,	'b','q',	/* chess black queen */
	0x265c,	'b','r',	/* chess black rook */
	0x265d,	'b','b',	/* chess black bishop */
	0x265e,	'b','n',	/* chess black knight */
	0x265f,	'b','p',	/* chess black pawn */
	0x03b1,	'*','a',	/* alpha */
	0x03b2,	'*','b',	/* beta */
	0x03b3,	'*','g',	/* gamma */
	0x03b4,	'*','d',	/* delta */
	0x03b5,	'*','e',	/* epsilon */
	0x03b6,	'*','z',	/* zeta */
	0x03b7,	'*','y',	/* eta */
	0x03b8,	'*','h',	/* theta */
	0x03b9,	'*','i',	/* iota */
	0x03ba,	'*','k',	/* kappa */
	0x03bb,	'*','l',	/* lambda */
	0x03bc,	'*','m',	/* mu */
	0x03bd,	'*','n',	/* nu */
	0x03be,	'*','c',	/* xsi */
	0x03bf,	'*','o',	/* omicron */
	0x03c0,	'*','p',	/* pi */
	0x03c1,	'*','r',	/* rho */
	0x03c2,	't','s',	/* terminal sigma */
	0x03c3,	'*','s',	/* sigma */
	0x03c4,	'*','t',	/* tau */
	0x03c5,	'*','u',	/* upsilon */
	0x03c6,	'*','f',	/* phi */
	0x03c7,	'*','x',	/* chi */
	0x03c8,	'*','q',	/* psi */
	0x03c9,	'*','w',	/* omega */	
	0x0391,	'*','A',	/* Alpha */
	0x0392,	'*','B',	/* Beta */
	0x0393,	'*','G',	/* Gamma */
	0x0394,	'*','D',	/* Delta */
	0x0395,	'*','E',	/* Epsilon */
	0x0396,	'*','Z',	/* Zeta */
	0x0397,	'*','Y',	/* Eta */
	0x0398,	'*','H',	/* Theta */
	0x0399,	'*','I',	/* Iota */
	0x039a,	'*','K',	/* Kappa */
	0x039b,	'*','L',	/* Lambda */
	0x039c,	'*','M',	/* Mu */
	0x039d,	'*','N',	/* Nu */
	0x039e,	'*','C',	/* Xsi */
	0x039f,	'*','O',	/* Omicron */
	0x03a0,	'*','P',	/* Pi */
	0x03a1,	'*','R',	/* Rho */
	0x03a3,	'*','S',	/* Sigma */
	0x03a4,	'*','T',	/* Tau */
	0x03a5,	'*','U',	/* Upsilon */
	0x03a6,	'*','F',	/* Phi */
	0x03a7,	'*','X',	/* Chi */
	0x03a8,	'*','Q',	/* Psi */
	0x03a9,	'*','W',	/* Omega */
	0x2190,	'<','-',	/* left arrow */
	0x2191,	'u','a',	/* up arrow */
	0x2192,	'-','>',	/* right arrow */
	0x2193,	'd','a',	/* down arrow */
	0x2194,	'a','b',	/* arrow both */
	0x21d0,	'V','=',	/* left double-line arrow */
	0x21d2,	'=','V',	/* right double-line arrow */
	0x2200,	'f','a',	/* forall */
	0x2203,	't','e',	/* there exists */
	0x2202,	'p','d',	/* partial differential */
	0x2205,	'e','s',	/* empty set */
	0x2206,	'D','e',	/* delta */
	0x2207,	'g','r',	/* gradient */
	0x2208,	'm','o',	/* element of */
	0x2209,	'!','m',	/* not element of */
	0x220d,	's','t',	/* such that */
	0x2217,	'*','*',	/* math asterisk */
	0x2219,	'b','u',	/* bullet */
	0x221a,	's','r',	/* radical */
	0x221d,	'p','t',	/* proportional */
	0x221e,	'i','f',	/* infinity */
	0x2220,	'a','n',	/* angle */
	0x2227,	'l','&',	/* logical and */
	0x2228,	'l','|',	/* logical or */
	0x2229,	'c','a',	/* intersection */
	0x222a,	'c','u',	/* union */
	0x222b,	'i','s',	/* integral */
	0x2234,	't','f',	/* therefore */
	0x2243,	'~','=',	/* asymptotically equal */
	0x2245,	'c','g',	/* congruent */
	0x2248,	'~','~',	/* almost equal */
	0x2260,	'!','=',	/* not equal */
	0x2261,	'=','=',	/* equivalent */
	0x2266,	'<','=',	/* less than or equal */
	0x2267,	'>','=',	/* greater than or equal */
	0x2282,	's','b',	/* proper subset */
	0x2283,	's','p',	/* proper superset */
	0x2284,	'!','b',	/* not subset */
	0x2286,	'i','b',	/* reflexive subset */
	0x2287,	'i','p',	/* reflexive superset */
	0x2295,	'O','+',	/* circle plus */
	0x2296,	'O','-',	/* circle minus */
	0x2297,	'O','x',	/* circle multiply */
	0x22a2,	't','u',	/* turnstile */
	0x22a8,	'T','u',	/* valid */
	0x22c4,	'l','z',	/* lozenge */
	0x22ef,	'e','l',	/* ellipses */
 	0x2639, ':','(',	/* saddy */
 	0x263a, ':',')',	/* white-face smiley */
 	0x263b, ';',')',	/* dark-face smiley */
	0,	0,
};

struct latin *mappings = NULL;

void
freelatin(void)
{
	free(mappings);
}

void
initlatin(void)
{
	FILE *keyboard = NULL;
	if (getenv("HOME"))
	{
		char path[1024] = {0};
		snprintf(path, 1023, "%s/.keyboard", getenv("HOME"));
		keyboard = fopen(path, "r");
	}

	if (!keyboard)
	{
		mappings = latintab;
		return;
	}

	mappings = calloc(MAPPING_MAX + 1, sizeof(struct latin));
	if (!mappings)
	{
		mappings = latintab;
		fclose(keyboard);
		return;
	}

	int j = 0;
	while (j < MAPPING_MAX)
	{
		int count = fscanf(keyboard, " %c%c %hx%*[^\n]\n", &(mappings[j].c[0]), &(mappings[j].c[1]), &(mappings[j].l));
		if (count == 3)
		{
			j++;

		}
		else if (count == EOF)
		{
			memset(&(mappings[j]), 0, sizeof(struct latin));
			break;
		}
		else
		{
			memset(&(mappings[j]), 0, sizeof(struct latin));
		}
	}

	fclose(keyboard);
	atexit(freelatin);
}

long
latin1(unsigned char *k)
{
	struct latin *l;

	for(l=mappings; l->l; l++)
		if(k[0]==l->c[0] && k[1]==l->c[1])
			return l->l;
	return -1;
}

int
unicode(unsigned char *k)
{
	int i, c;

	k++;	/* skip 'X' */
	c = 0;
	for(i=0; i<4; i++,k++){
		c <<= 4;
		if('0'<=*k && *k<='9')
			c += *k-'0';
		else if('a'<=*k && *k<='f')
			c += 10 + *k-'a';
		else if('A'<=*k && *k<='F')
			c += 10 + *k-'A';
		else
			return -1;
	}
	return c;
}
