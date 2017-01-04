/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>
#include "flayer.h"
#include "samterm.h"

int cursorfd;
int input;
int got;
int block;
Keystroke   keystroke;
int reshaped;
uint8_t   *hostp;
uint8_t   *hoststop;
uint8_t   *externbase;
uint8_t   *externp;
uint8_t   *externstop;
void    panic(char*);

void
initio(void){
    extern int exfd;

    einit(Emouse|Ekeyboard);
    estart(Ehost, 0, 0, false);
    if (exfd >= 0)
        estart(Eextern, exfd, 8192, true);
}

void
frgetmouse(void)
{
    mouse = emouse();
}

void
mouseunblock(void)
{
    got &= ~Emouse;
}

void
kbdblock(void)
{       /* ca suffit */
    block = Ekeyboard|Eextern;
}

int
button(int but)
{
    frgetmouse();
    return mouse.buttons&(1<<(but-1));
}

void
externload(Event *e)
{
    externbase = malloc(e->n);
    if(externbase == 0)
        return;
    memmove(externbase, e->data, e->n);
    externp = externbase;
    externstop = externbase + e->n;
    got |= Eextern;
}

int
waitforio(void)
{
    uint64_t type;
    static Event e;

    if(got & ~block)
        return got & ~block;
    type = eread(~(got|block), &e);
    switch(type){
    case Ehost:
        hostp = e.data;
        hoststop = hostp + e.n;
        block = 0;
        break;
    case Eextern:
        externload(&e);
        break;
    case Ekeyboard:
        keystroke = e.keystroke;
        break;
    case Emouse:
        mouse = e.mouse;
        break;
    }
    got |= type;
    return got; 
}

int
rcvchar(void)
{
    int c;

    if(!(got & Ehost))
        return -1;
    c = *hostp++;
    if(hostp == hoststop)
        got &= ~Ehost;
    return c;
}

char*
rcvstring(void)
{
    *hoststop = 0;
    got &= ~Ehost;
    return (char*)hostp;
}

int
getch(void)
{
    int c;

    while((c = rcvchar()) == -1){
        block = ~Ehost;
        waitforio();
        block = 0;
    }
    return c;
}

int
externchar(void)
{
    wchar_t r;

    loop:
    if(got & (Eextern & ~block)){
        externp += chartorune(&r, (char*)externp);
        if(externp >= externstop){
            got &= ~Eextern;
            free(externbase);
        }
        if(r == 0)
            goto loop;
        return r;
    }
    return -1;
}

Keystroke
qpeekc(void)
{
    return keystroke;
}

Keystroke
kbdchar(void)
{
    Keystroke k = {0};
    static Event e;

    k.c = externchar();
    if(k.c > 0)
        return k;
    if(got & Ekeyboard){
        k = keystroke;
        memset(&keystroke, 0, sizeof(keystroke));
        got &= ~Ekeyboard;
        return k;
    }
    while(ecanread(Eextern)){
        eread(Eextern, &e);
        externload(&e);
        k.c = externchar();
        if(k.c > 0)
            return k;
    }
    if(!ecankbd()){
        k.c = -1;
        return k;
    }
    return ekbd();
}

void
ereshaped(Rectangle r)
{
    reshaped = 1;
}

int
RESHAPED(void)
{
    if(reshaped){
        screen.r = bscreenrect(&screen.clipr);
        reshaped = 0;
        return 1;
    }
    return 0;
}

void
mouseexit(void)
{
    exit(EXIT_SUCCESS);
}
