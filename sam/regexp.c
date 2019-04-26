/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include "sam.h"

Rangeset    sel;
String      lastregexp;

/*
 * Machine Information
 */
typedef struct Inst Inst;

struct Inst
{
    int64_t    type;   /* < 0x100000000 ==> literal, otherwise action */
    union {
        int rsubid;
        int class;              /* index into `wchar_t **class` */
        struct Inst *rother;
        struct Inst *rright;
    } r;
    union{
        struct Inst *lleft;
        struct Inst *lnext;
    } l;
};
#define sid r.rsid
#define subid   r.rsubid
#define rclass  r.class
#define other   r.rother
#define right   r.rright
#define left    l.lleft
#define next    l.lnext

#define NPROG   1024
Inst    program[NPROG];
Inst    *progp;
Inst    *startinst; /* First inst. of program; might not be program[0] */
Inst    *bstartinst;    /* same for backwards machine */

typedef struct Ilist Ilist;
struct Ilist
{
    Inst    *inst;      /* Instruction of the thread */
    Rangeset se;
    Posn    startp;     /* first char of match */
};

#define NLIST   128

Ilist   *tl, *nl;   /* This list, next list */
Ilist   list[2][NLIST];
static  Rangeset sempty;

/*
 * Actions and Tokens
 *
 *  0x1000000xx are operators, value == precedence
 *  0x2000000xx are tokens, i.e. operands for operators
 */
#define OPERATOR    0x100000000 /* Bitmask of all operators */
#define START       0x100000000 /* Start, used for marker on stack */
#define RBRA        0x100000001 /* Right bracket, ) */
#define LBRA        0x100000002 /* Left bracket, ( */
#define OR          0x100000003 /* Alternation, | */
#define CAT         0x100000004 /* Concatentation, implicit operator */
#define STAR        0x100000005 /* Closure, * */
#define PLUS        0x100000006 /* a+ == aa* */
#define QUEST       0x100000007 /* a? == a|nothing, i.e. 0 or 1 a's */
#define ANY         0x200000000 /* Any character but newline, . */
#define NOP         0x200000001 /* No operation, internal use only */
#define BOL         0x200000002 /* Beginning of line, ^ */
#define EOL         0x200000003 /* End of line, $ */
#define CCLASS      0x200000004 /* Character class, [] */
#define NCCLASS     0x200000005 /* Negated character class, [^] */
#define END         0x200000077 /* Terminate: match found */

#define ISATOR      0x100000000
#define ISAND       0x200000000

/*
 * Parser Information
 */
typedef struct Node Node;
struct Node
{
    Inst    *first;
    Inst    *last;
};

#define NSTACK  20
Node    andstack[NSTACK];
Node    *andp;
int64_t atorstack[NSTACK];
int64_t *atorp;
bool lastwasand; /* Last token was operand */
int cursubid;
int subidstack[NSTACK];
int *subidp;
bool backwards;
int nbra;
wchar_t    *exprp;     /* pointer to next character in source expression */
#define DCLASS  10  /* allocation increment */
int nclass;     /* number active */
int Nclass;     /* high water mark */
wchar_t    **class;
bool negateclass;

void    addinst(Ilist *l, Inst *inst, Rangeset *sep);
void    newmatch(Rangeset*);
void    bnewmatch(Rangeset*);
void    pushand(Inst*, Inst*);
void    pushator(int64_t);
Node    *popand(int);
int64_t popator(void);
void    startlex(wchar_t*);
int64_t lex(void);
void    operator(int64_t);
void    operand(int64_t);
void    evaluntil(int64_t);
void    optimize(Inst*);
void    bldcclass(void);

void
regerror(Err e)
{
    Strzero(&lastregexp);
    error(e);
}

void
regerror_c(Err e, int c)
{
    Strzero(&lastregexp);
    error_c(e, c);
}

Inst *
newinst(int64_t t)
{
    if(progp >= &program[NPROG])
        regerror(Etoolong);
    progp->type = t;
    progp->left = 0;
    progp->right = 0;
    return progp++;
}

Inst *
realcompile(wchar_t *s)
{
    int64_t token;

    startlex(s);
    atorp = atorstack;
    andp = andstack;
    subidp = subidstack;
    cursubid = 0;
    lastwasand = false;
    /* Start with a low priority operator to prime parser */
    pushator(START-1);
    while((token=lex()) != END){
        if((token&ISATOR) == OPERATOR)
            operator(token);
        else
            operand(token);
    }
    /* Close with a low priority operator */
    evaluntil(START);
    /* Force END */
    operand(END);
    evaluntil(START);
    if(nbra)
        regerror(Eleftpar);
    --andp; /* points to first and only operand */
    return andp->first;
}

void
compile(String *s)
{
    int i;
    Inst *oprogp;

    if(Strcmp(s, &lastregexp)==0)
        return;
    for(i=0; i<nclass; i++)
        free(class[i]);
    nclass = 0;
    progp = program;
    backwards = false;
    startinst = realcompile(s->s);
    optimize(program);
    oprogp = progp;
    backwards = true;
    bstartinst = realcompile(s->s);
    optimize(oprogp);
    Strduplstr(&lastregexp, s);
}

void
operand(int64_t t)
{
    Inst *i;
    if(lastwasand)
        operator(CAT);  /* catenate is implicit */
    i = newinst(t);
    if(t == CCLASS){
        if(negateclass)
            i->type = NCCLASS;  /* UGH */
        i->rclass = nclass-1;       /* UGH */
    }
    pushand(i, i);
    lastwasand = true;
}

void
operator(int64_t t)
{
    if(t==RBRA && --nbra<0)
        regerror(Erightpar);
    if(t==LBRA){
        cursubid++; /* silently ignored */
        nbra++;
        if(lastwasand)
            operator(CAT);
    }else
        evaluntil(t);
    if(t!=RBRA)
        pushator(t);
    lastwasand = false;
    if(t==STAR || t==QUEST || t==PLUS || t==RBRA)
        lastwasand = true;  /* these look like operands */
}

void
cant(char *s)
{
    char buf[100];

    snprintf(buf, sizeof(buf) - 1, "regexp: can't happen: %s", s);
    panic(buf);
}

void
pushand(Inst *f, Inst *l)
{
    if(andp >= &andstack[NSTACK])
        cant("operand stack overflow");
    andp->first = f;
    andp->last = l;
    andp++;
}

void
pushator(int64_t t)
{
    if(atorp >= &atorstack[NSTACK])
        cant("operator stack overflow");
    *atorp++=t;
    if(cursubid >= NSUBEXP)
        *subidp++= -1;
    else
        *subidp++=cursubid;
}

Node *
popand(int op)
{
    if(andp <= &andstack[0]){
        if(op)
            regerror_c(Emissop, op);
        else
            regerror(Ebadregexp);
    }
    return --andp;
}

int64_t
popator(void)
{
    if(atorp <= &atorstack[0])
        cant("operator stack underflow");
    --subidp;
    return *--atorp;
}

void
evaluntil(int64_t pri)
{
    Node *op1, *op2, *t;
    Inst *inst1, *inst2;

    while(pri==RBRA || atorp[-1]>=pri){
        switch(popator()){
        case LBRA:
            op1 = popand('(');
            inst2 = newinst(RBRA);
            inst2->subid = *subidp;
            op1->last->next = inst2;
            inst1 = newinst(LBRA);
            inst1->subid = *subidp;
            inst1->next = op1->first;
            pushand(inst1, inst2);
            return;     /* must have been RBRA */
        default:
            panic("unknown regexp operator");
            break;
        case OR:
            op2 = popand('|');
            op1 = popand('|');
            inst2 = newinst(NOP);
            op2->last->next = inst2;
            op1->last->next = inst2;
            inst1 = newinst(OR);
            inst1->right = op1->first;
            inst1->left = op2->first;
            pushand(inst1, inst2);
            break;
        case CAT:
            op2 = popand(0);
            op1 = popand(0);
            if(backwards && op2->first->type!=END)
                t = op1, op1 = op2, op2 = t;
            op1->last->next = op2->first;
            pushand(op1->first, op2->last);
            break;
        case STAR:
            op2 = popand('*');
            inst1 = newinst(OR);
            op2->last->next = inst1;
            inst1->right = op2->first;
            pushand(inst1, inst1);
            break;
        case PLUS:
            op2 = popand('+');
            inst1 = newinst(OR);
            op2->last->next = inst1;
            inst1->right = op2->first;
            pushand(op2->first, inst1);
            break;
        case QUEST:
            op2 = popand('?');
            inst1 = newinst(OR);
            inst2 = newinst(NOP);
            inst1->left = inst2;
            inst1->right = op2->first;
            op2->last->next = inst2;
            pushand(inst1, inst2);
            break;
        }
    }
}


void
optimize(Inst *start)
{
    Inst *inst, *target;

    for(inst=start; inst->type!=END; inst++){
        target = inst->next;
        while(target->type == NOP)
            target = target->next;
        inst->next = target;
    }
}

#ifdef  DEBUG
void
dumpstack(void){
    Node *stk;
    int *ip;

    dprint(L"operators\n");
    for(ip = atorstack; ip<atorp; ip++)
        dprint(L"0%o\n", *ip);
    dprint(L"operands\n");
    for(stk = andstack; stk<andp; stk++)
        dprint(L"0%o\t0%o\n", stk->first->type, stk->last->type);
}
void
dump(void){
    Inst *l;

    l = program;
    do{
        dprint(L"%d:\t0%o\t%d\t%d\n", l-program, l->type,
            l->left-program, l->right-program);
    }while(l++->type);
}
#endif

void
startlex(wchar_t *s)
{
    exprp = s;
    nbra = 0;
}


int64_t
lex(void){
    int64_t c= *exprp++;

    switch(c){
    case '\\':
        if(*exprp)
            if((c= *exprp++)=='n')
                c='\n';
        break;
    case 0:
        c = END;
        --exprp;    /* In case we come here again */
        break;
    case '*':
        c = STAR;
        break;
    case '?':
        c = QUEST;
        break;
    case '+':
        c = PLUS;
        break;
    case '|':
        c = OR;
        break;
    case '.':
        c = ANY;
        break;
    case '(':
        c = LBRA;
        break;
    case ')':
        c = RBRA;
        break;
    case '^':
        c = BOL;
        break;
    case '$':
        c = EOL;
        break;
    case '[':
        c = CCLASS;
        bldcclass();
        break;
    }
    return c;
}

int64_t
nextrec(void){
    if(exprp[0]==0 || (exprp[0]=='\\' && exprp[1]==0))
        regerror(Ebadclass);
    if(exprp[0] == '\\'){
        exprp++;
        if(*exprp=='n'){
            exprp++;
            return '\n';
        }
        return *exprp++|0x100000000;
    }
    return *exprp++;
}

void
bldcclass(void)
{
    int64_t c1, c2, n, na;
    wchar_t *classp;

    classp = emalloc(DCLASS*RUNESIZE);
    n = 0;
    na = DCLASS;
    /* we have already seen the '[' */
    if(*exprp == '^'){
        classp[n++] = '\n'; /* don't match newline in negate case */
        negateclass = true;
        exprp++;
    }else
        negateclass = false;
    while((c1 = nextrec()) != ']'){
        if(c1 == '-'){
    Error:
            free(classp);
            regerror(Ebadclass);
        }
        if(n+4 >= na){      /* 3 runes plus NUL */
            na += DCLASS;
            classp = erealloc(classp, na*RUNESIZE);
        }
        if(*exprp == '-'){
            exprp++;    /* eat '-' */
            if((c2 = nextrec()) == ']')
                goto Error;
            classp[n+0] = 0xFFFFFFFF;
            classp[n+1] = c1;
            classp[n+2] = c2;
            n += 3;
        }else
            classp[n++] = c1;
    }
    classp[n] = 0;
    if(nclass == Nclass){
        Nclass += DCLASS;
        class = erealloc(class, Nclass*sizeof(wchar_t*));
    }
    class[nclass++] = classp;
}

bool
classmatch(int classno, wchar_t c, bool negate)
{
    wchar_t *p;

    p = class[classno];
    while(*p){
        if(*p == 0xFFFFFFFF){
            if(p[1]<=c && c<=p[2])
                return !negate;
            p += 3;
        }else if(*p++ == c)
            return !negate;
    }
    return negate;
}

/*
 * Note optimization in addinst:
 *  *l must be pending when addinst called; if *l has been looked
 *      at already, the optimization is a bug.
 */
void
addinst(Ilist *l, Inst *inst, Rangeset *sep)
{
    Ilist *p;

    for(p = l; p->inst; p++){
        if(p->inst==inst){
            if((sep)->p[0].p1 < p->se.p[0].p1)
                p->se= *sep;    /* this would be bug */
            return; /* It's already there */
        }
    }
    p->inst = inst;
    p->se= *sep;
    (p+1)->inst = 0;
}

int
execute(File *f, Posn startp, Posn eof)
{
    int flag = 0;
    Inst *inst;
    Ilist *tlp;
    Posn p = startp;
    int nnl = 0, ntl;
    int c;
    int wrapped = 0;
    int startchar = startinst->type<OPERATOR? startinst->type : 0;

    list[0][0].inst = list[1][0].inst = 0;
    sel.p[0].p1 = -1;
    Fgetcset(f, startp);
    /* Execute machine once for each character */
    for(;;p++){
    doloop:
        c = Fgetc(f);
        if(p>=eof || c<0){
            switch(wrapped++){
            case 0:     /* let loop run one more click */
            case 2:
                break;
            case 1:     /* expired; wrap to beginning */
                if(sel.p[0].p1>=0 || eof!=INFINITY)
                    goto Return;
                list[0][0].inst = list[1][0].inst = 0;
                Fgetcset(f, (Posn)0);
                p = 0;
                goto doloop;
            default:
                goto Return;
            }
        }else if(((wrapped && p>=startp) || sel.p[0].p1>0) && nnl==0)
            break;
        /* fast check for first char */
        if(startchar && nnl==0 && c!=startchar)
            continue;
        tl = list[flag];
        nl = list[flag^=1];
        nl->inst = 0;
        ntl = nnl;
        nnl = 0;
        if(sel.p[0].p1<0 && (!wrapped || p<startp || startp==eof)){
            /* Add first instruction to this list */
            if(++ntl >= NLIST)
    Overflow:
                error(Eoverflow);
            sempty.p[0].p1 = p;
            addinst(tl, startinst, &sempty);
        }
        /* Execute machine until this list is empty */
        for(tlp = tl; (inst = tlp->inst); tlp++){ /* assignment = */
    Switchstmt:
            switch(inst->type){
            default:    /* regular character */
                if(inst->type==c){
    Addinst:
                    if(++nnl >= NLIST)
                        goto Overflow;
                    addinst(nl, inst->next, &tlp->se);
                }
                break;
            case LBRA:
                if(inst->subid>=0)
                    tlp->se.p[inst->subid].p1 = p;
                inst = inst->next;
                goto Switchstmt;
            case RBRA:
                if(inst->subid>=0)
                    tlp->se.p[inst->subid].p2 = p;
                inst = inst->next;
                goto Switchstmt;
            case ANY:
                if(c!='\n')
                    goto Addinst;
                break;
            case BOL:
                if(p == 0){
    Step:
                    inst = inst->next;
                    goto Switchstmt;
                }
                if(f->getci > 1){
                    if(f->getcbuf[f->getci-2]=='\n')
                        goto Step;
                }else{
                    wchar_t c;
                    if(Fchars(f, &c, p-1, p)==1 && c=='\n')
                        goto Step;
                }
                break;
            case EOL:
                if(c == '\n')
                    goto Step;
                break;
            case CCLASS:
                if(c>=0 && classmatch(inst->rclass, c, 0))
                    goto Addinst;
                break;
            case NCCLASS:
                if(c>=0 && classmatch(inst->rclass, c, 1))
                    goto Addinst;
                break;
            case OR:
                /* evaluate right choice later */
                if(++ntl >= NLIST)
                    goto Overflow;
                addinst(tlp, inst->right, &tlp->se);
                /* efficiency: advance and re-evaluate */
                inst = inst->left;
                goto Switchstmt;
            case END:   /* Match! */
                tlp->se.p[0].p2 = p;
                newmatch(&tlp->se);
                break;
            }
        }
    }
    Return:
    return sel.p[0].p1>=0;
}

void
newmatch(Rangeset *sp)
{
    int i;

    if(sel.p[0].p1<0 || sp->p[0].p1<sel.p[0].p1 ||
       (sp->p[0].p1==sel.p[0].p1 && sp->p[0].p2>sel.p[0].p2))
        for(i = 0; i<NSUBEXP; i++)
            sel.p[i] = sp->p[i];
}

int
bexecute(File *f, Posn startp)
{
    int flag = 0;
    Inst *inst;
    Ilist *tlp;
    Posn p = startp;
    int nnl = 0, ntl;
    int c;
    int wrapped = 0;
    int startchar = bstartinst->type<OPERATOR? bstartinst->type : 0;

    list[0][0].inst = list[1][0].inst = 0;
    sel.p[0].p1= -1;
    Fgetcset(f, startp);
    /* Execute machine once for each character, including terminal NUL */
    for(;;--p){
    doloop:
        if((c = Fbgetc(f))==-1){
            switch(wrapped++){
            case 0:     /* let loop run one more click */
            case 2:
                break;
            case 1:     /* expired; wrap to end */
                if(sel.p[0].p1>=0)
            case 3:
                    goto Return;
                list[0][0].inst = list[1][0].inst = 0;
                Fgetcset(f, f->nrunes);
                p = f->nrunes;
                goto doloop;
            default:
                goto Return;
            }
        }else if(((wrapped && p<=startp) || sel.p[0].p1>0) && nnl==0)
            break;
        /* fast check for first char */
        if(startchar && nnl==0 && c!=startchar)
            continue;
        tl = list[flag];
        nl = list[flag^=1];
        nl->inst = 0;
        ntl = nnl;
        nnl = 0;
        if(sel.p[0].p1<0 && (!wrapped || p>startp)){
            /* Add first instruction to this list */
            if(++ntl >= NLIST)
    Overflow:
                error(Eoverflow);
            /* the minus is so the optimizations in addinst work */
            sempty.p[0].p1 = -p;
            addinst(tl, bstartinst, &sempty);
        }
        /* Execute machine until this list is empty */
        for(tlp = tl; (inst = tlp->inst); tlp++){ /* assignment = */
    Switchstmt:
            switch(inst->type){
            default:    /* regular character */
                if(inst->type == c){
    Addinst:
                    if(++nnl >= NLIST)
                        goto Overflow;
                    addinst(nl, inst->next, &tlp->se);
                }
                break;
            case LBRA:
                if(inst->subid>=0)
                    tlp->se.p[inst->subid].p1 = p;
                inst = inst->next;
                goto Switchstmt;
            case RBRA:
                if(inst->subid >= 0)
                    tlp->se.p[inst->subid].p2 = p;
                inst = inst->next;
                goto Switchstmt;
            case ANY:
                if(c != '\n')
                    goto Addinst;
                break;
            case BOL:
                if(c=='\n' || p==0){
    Step:
                    inst = inst->next;
                    goto Switchstmt;
                }
                break;
            case EOL:
                if(f->getci<f->ngetc-1){
                    if(f->getcbuf[f->getci+1]=='\n')
                        goto Step;
                }else if(p<f->nrunes-1){
                    wchar_t c;
                    if(Fchars(f, &c, p, p+1)==1 && c=='\n')
                        goto Step;
                }
                break;
            case CCLASS:
                if(c>=0 && classmatch(inst->rclass, c, 0))
                    goto Addinst;
                break;
            case NCCLASS:
                if(c>=0 && classmatch(inst->rclass, c, 1))
                    goto Addinst;
                break;
            case OR:
                /* evaluate right choice later */
                if(++ntl >= NLIST)
                    goto Overflow;
                addinst(tlp, inst->right, &tlp->se);
                /* efficiency: advance and re-evaluate */
                inst = inst->left;
                goto Switchstmt;
            case END:   /* Match! */
                tlp->se.p[0].p1 = -tlp->se.p[0].p1; /* minus sign */
                tlp->se.p[0].p2 = p;
                bnewmatch(&tlp->se);
                break;
            }
        }
    }
    Return:
    return sel.p[0].p1>=0;
}

void
bnewmatch(Rangeset *sp)
{
        int  i;
        if(sel.p[0].p1<0 || sp->p[0].p1>sel.p[0].p2 || (sp->p[0].p1==sel.p[0].p2 && sp->p[0].p2<sel.p[0].p1))
                for(i = 0; i<NSUBEXP; i++){       /* note the reversal; p1<=p2 */
                        sel.p[i].p1 = sp->p[i].p2;
                        sel.p[i].p2 = sp->p[i].p1;
                }
}
