/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
/* Copyright (c) 2016 Rob King */

#define _XOPEN_SOURCE 500
#include "sam.h"

#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

wchar_t    genbuf[BLOCKSIZE];
FILE *io;
bool panicking;
bool rescuing;
Mod modnum;
String  genstr;
String  rhs;
String  wd;
String  cmdstr;
wchar_t    empty[] = { 0 };
char    *genc;
File    *curfile;
File    *flist;
File    *cmd;
jmp_buf mainloop;
List tempfile;
bool quitok = true;
bool downloaded;
bool expandtabs;
bool dflag;
char    *machine;
char    *home;
bool bpipeok;
int termlocked;
char    *samterm = "samterm";
char    *rsamname = "sam";
char *sh = "sh";
char *shpath = "/bin/sh";
char *rmsocketname = NULL;

wchar_t    baddir[] = { '<', 'b', 'a', 'd', 'd', 'i', 'r', '>', '\n'};

void    usage(void);

static void
hup(int sig)
{
    rescue();
    exit(EXIT_FAILURE);
}

int sammain(int argc, char *argv[]);
int bmain(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    if (strcmp(basename(argv[0]), "B") == 0)
        return bmain(argc, argv);
    return sammain(argc, argv);
}

#define B_CMD_MAX 4095
const char *
getbsocketname(const char *machine)
{
    const char *user = getenv("USER")? getenv("USER") : getenv("LOGNAME")? getenv("LOGNAME") : "nemo";
    const char *path = getenv("SAMSOCKETPATH")? getenv("SAMSOCKETPATH") : getenv("HOME");
    static char name[FILENAME_MAX + 1] = {0};

    if (getenv("SAMSOCKETNAME"))
        return getenv("SAMSOCKETNAME");

    if (name[0])
        return name;

    snprintf(name, FILENAME_MAX, "%s/.sam.%s", path, machine);
    if (access(name, R_OK) == 0)
        return name;

    snprintf(name, FILENAME_MAX, "%s/.sam.remote.%s", path, user);
    if (access(name, R_OK) == 0)
        return name;

    snprintf(name, FILENAME_MAX, "/tmp/sam.remote.%s", user);
    if (access(name, R_OK) == 0)
        return name;

    return NULL;
}

int
bmain(int argc, char *argv[])
{
    int fd, o;
    struct sockaddr_un un = {0};
    char cmd[B_CMD_MAX + 1] = {0};
    bool machineset = false;

    machine = "localhost";
    while ((o = getopt(argc, argv, "r:")) != -1){
        switch (o){
            case 'r':
                machine = optarg;
                machineset = true;
                break;

            default:
                return fputs("usage: B [-r MACHINE] FILE...\n", stderr), EXIT_FAILURE;
        }
    }

    if (getbsocketname(machine) == NULL){
        fputs("could not determine controlling socket name\n", stderr);
        if (!machineset)
            machine = NULL;
        return sammain(argc, argv);
    }

    argc -= optind;
    argv += optind;

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, getbsocketname(machine), sizeof(un.sun_path) - 1);
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 || connect(fd, (struct sockaddr *)&un, sizeof(un)) < 0)
        return perror("could not open socket"), EXIT_FAILURE;

    strncat(cmd, "B ", B_CMD_MAX);
    for (int i = 0; i < argc; i++){
        char path[FILENAME_MAX + 1];
        if (realpath(argv[i], path) == NULL)
            perror(argv[i]);
        else{
            strncat(cmd, " ", B_CMD_MAX);
            strncat(cmd, path, B_CMD_MAX);
        }
    }
    strncat(cmd, "\n", B_CMD_MAX);

    if (write(fd, cmd, strlen(cmd)) <= 0)
        return perror("could not send command"), EXIT_FAILURE;

    close(fd);
    return EXIT_SUCCESS;
}

void
rmsocket(void)
{
    if (rmsocketname)
        unlink(rmsocketname);
}

int
sammain(int argc, char *argv[])
{
    bool trylock = true;
    int i, o;
    String *t;
    char *arg[argc + 1], **ap;

    ap = &arg[argc];
    arg[0] = "samterm";
    setlocale(LC_ALL, "");

    while ((o = getopt(argc, argv, "SedR:r:t:s:")) != -1){
        switch (o){
            case 'd':
                dflag = true;
                break;

            case 'r':
                machine = optarg;
                break;

            case 'R':
                rmsocketname = optarg;
                atexit(rmsocket);
                break;

            case 't':
                samterm = optarg;
                break;

            case 's':
                rsamname = optarg;
                break;

            case 'S':
                trylock = false;
                break;

            default:
                usage();
        }
    }

    if (trylock && !canlocksocket(machine) && !dflag)
        return bmain(argc, argv);

    argv += optind;
    argc -= optind;

    Strinit(&cmdstr);
    Strinit0(&lastpat);
    Strinit0(&lastregexp);
    Strinit0(&genstr);
    Strinit0(&rhs);
    Strinit0(&wd);
    Strinit0(&plan9cmd);

    tempfile.listptr = emalloc(0);
    home = getenv("HOME") ? getenv("HOME") : "/";
    shpath = getenv("SHELL") ? getenv("SHELL") : shpath;
    sh = basename(shpath);
    if(!dflag)
        startup(machine, rmsocketname != NULL, trylock);
    Fstart();

    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, hup);
    signal(SIGPIPE, SIG_IGN);

    if(argc > 0){
        for(i=0; i<argc; i++)
            if(!setjmp(mainloop)){
                t = tmpcstr(argv[i]);
                Straddc(t, '\0');
                Strduplstr(&genstr, t);
                freetmpstr(t);
                Fsetname(newfile(), &genstr);
            }
    }else if(!downloaded)
        newfile()->state = Clean;
    modnum++;
    if(file.nused)
        current(file.filepptr[0]);

    atexit(scram);
    setjmp(mainloop);
    cmdloop();

    trytoquit();    /* if we already q'ed, quitok will be true */

    exit(EXIT_SUCCESS);
}

void
scram(void)
{
    freecmd();
    for (int i = 0; i < file.nused; i++)
        Fclose(file.filepptr[i]);

    if (!downloaded)
        Fclose(cmd);

    if (genc)
        free(genc);

    Strclose(&cmdstr);
    Strclose(&lastpat);
    Strclose(&lastregexp);
    Strclose(&genstr);
    Strclose(&rhs);
    Strclose(&wd);
    Strclose(&plan9cmd);

    if (file.listptr)
        free(file.listptr);

    if (tempfile.listptr)
        free(tempfile.listptr);

    freecmdlists();
    freebufs();
}

void
usage(void)
{
    fprintf(stderr, "usage: sam [-r machine] [-dfeS] [-t samterm] [-s samname] FILE...\n");
    exit(EXIT_FAILURE);
}

void
rescue(void)
{
    if (rescuing++)
        return;

    if (io){
        fclose(io);
        io = NULL;
    }

    for (int i = 0; i < file.nused; i++){
        char buf[FILENAME_MAX + 1] = {0};
        File *f = file.filepptr[i];
        if (f == cmd || f->nrunes == 0 || f->state != Dirty)
            continue;

        snprintf(buf, FILENAME_MAX, "%s/sam.save", home);
        if ((io = fopen(buf, "w")) == NULL)
            continue;

        fprintf(io, "samsave() {\n"
                   "    echo \"${1}?\"\n"
                   "    read yn < /dev/tty\n"
                   "    case \"${yn}\" in\n"
                   "        [Yy]*) cat > \"${1}\"\n"
                   "    esac\n"
                   "}\n");

        if(f->name.s[0]){
            char *c = Strtoc(&f->name);
            snprintf(buf, FILENAME_MAX, "%s", c);
            free(c);
        }else
            snprintf(buf, FILENAME_MAX, "nameless.%d", i);

        fprintf(io, "samsave %s <<'---%s'\n", buf, buf);
        addr.r.p1 = 0, addr.r.p2 = f->nrunes;
        writeio(f);
        fprintf(io, "\n---%s\n", (char *)buf);
        flushio();
    }
}

void
panic(char *s)
{
    int wasd;

    if(!panicking++ && !setjmp(mainloop)){
        wasd = downloaded;
        downloaded = 0;
        dprint(L"sam: panic: %s\n", s);
        if(wasd)
            fprintf(stderr, "sam: panic: %s\n", s);
        rescue();
        abort();
    }
}

void
hiccough(char *s)
{
    if(rescuing)
        exit(EXIT_FAILURE);

    if(s)
        dprint(L"%s\n", s);

    resetcmd();
    resetxec();
    resetsys();

    if (io){
        fclose(io);
        io = NULL;
    }

    if(undobuf->nrunes)
        Bdelete(undobuf, (Posn)0, undobuf->nrunes);

    update();

    if (curfile) {
        if (curfile->state==Unread)
            curfile->state = Clean;
        else if (downloaded)
            outTs(Hcurrent, curfile->tag);
    }

    longjmp(mainloop, 1);
}

void
intr(void)
{
    error(Eintr);
}

void
trytoclose(File *f)
{
    char *t;
    char buf[256];

    if(f == cmd)    /* possible? */
        return;
    if(f->deleted)
        return;
    if(f->state==Dirty && !f->closeok){
        f->closeok = true;
        if(f->name.s[0]){
            t = Strtoc(&f->name);
            strncpy(buf, t, sizeof buf-1);
            free(t);
        }else
            strcpy(buf, "nameless file");
        error_s(Emodified, buf);
    }
    f->deleted = true;
}

void
trytoquit(void)
{
    int c;
    File *f;

    if (!quitok){
        for(c = 0; c < file.nused; c++){
            f = file.filepptr[c];
            if(f != cmd && f->state == Dirty){
                quitok = true;
                eof = false;
                error(Echanges);
            }
        }
    }
}

void
load(File *f)
{
    Address saveaddr;

    Strduplstr(&genstr, &f->name);
    filename(f);
    if(f->name.s[0]){
        saveaddr = addr;
        edit(f, 'I');
        addr = saveaddr;
    }else
        f->state = Clean;
    Fupdate(f, true, true);
}

void
cmdupdate(void)
{
    if(cmd && cmd->mod!=0){
        Fupdate(cmd, false, downloaded);
        cmd->dot.r.p1 = cmd->dot.r.p2 = cmd->nrunes;
        telldot(cmd);
    }
}

void
delete(File *f)
{
    if(downloaded && f->rasp)
        outTs(Hclose, f->tag);
    delfile(f);
    if(f == curfile)
        current(0);
}

void
update(void)
{
    int i, anymod;
    File *f;

    settempfile();
    for(anymod = i=0; i<tempfile.nused; i++){
        f = tempfile.filepptr[i];
        if(f==cmd)  /* cmd gets done in main() */
            continue;
        if(f->deleted) {
            delete(f);
            continue;
        }
        if(f->mod==modnum && Fupdate(f, false, downloaded))
            anymod++;
        if(f->rasp)
            telldot(f);
    }
    if(anymod)
        modnum++;
}

File *
current(File *f)
{
    return curfile = f;
}

void
edit(File *f, int cmd)
{
    bool empty = true;
    Posn p;
    bool nulls;

    if(cmd == 'r')
        Fdelete(f, addr.r.p1, addr.r.p2);
    if(cmd=='e' || cmd=='I'){
        Fdelete(f, (Posn)0, f->nrunes);
        addr.r.p2 = f->nrunes;
    }else if(f->nrunes!=0 || (f->name.s[0] && Strcmp(&genstr, &f->name)!=0))
        empty = false;
    if ((io = fopen(genc, "r")) == NULL) {
        if (curfile && curfile->state == Unread)
            curfile->state = Clean;
        error_s(Eopen, genc);
    }
    p = readio(f, &nulls, empty);
    if (nulls)
        warn(Wnulls);
    closeio((cmd=='e' || cmd=='I')? -1 : p);
    if(cmd == 'r')
        f->ndot.r.p1 = addr.r.p2, f->ndot.r.p2 = addr.r.p2+p;
    else
        f->ndot.r.p1 = f->ndot.r.p2 = 0;
    f->closeok = empty;
    if (quitok)
        quitok = empty;
    else
        quitok = false;
    state(f, empty? Clean : Dirty);
    if(cmd == 'e')
        filename(f);
}

int
getname(File *f, String *s, bool save)
{
    int c, i;

    Strzero(&genstr);
    if(genc){
        free(genc);
        genc = 0;
    }
    if(s==0 || (c = s->s[0])==0){       /* no name provided */
        if(f)
            Strduplstr(&genstr, &f->name);
        else
            Straddc(&genstr, '\0');
        goto Return;
    }
    if (c != L' ' && c != L'\t')
        error(Eblank);

    for (i = 0; (c = s->s[i]) == L' ' || c == L'\t'; i++)
        ;

    while (s->s[i] > L' ')
        Straddc(&genstr, s->s[i++]);
    if(s->s[i])
        error(Enewline);
    Straddc(&genstr, '\0');
    if(f && (save || f->name.s[0]==0)){
        Fsetname(f, &genstr);
        if(Strcmp(&f->name, &genstr)){
            quitok = (f->closeok = false);
            f->qid = 0;
            f->date = 0;
            state(f, Dirty); /* if it's 'e', fix later */
        }
    }
    Return:
    genc = Strtoc(&genstr);
    return genstr.n;  /* strlen(name) */
}

void
filename(File *f)
{
    if(genc)
        free(genc);
    genc = Strtoc(&f->name);
    dprint(L"%c%c%c %s\n", " '"[f->state==Dirty],
        "-+"[f->rasp!=0], " ."[f==curfile], genc);
}

void
undostep(File *f)
{
    Buffer *t;
    int changes;
    Mark mark;

    t = f->transcript;
    changes = Fupdate(f, true, true);
    Bread(t, (wchar_t*)&mark, (sizeof mark)/RUNESIZE, f->markp);
    Bdelete(t, f->markp, t->nrunes);
    f->markp = mark.p;
    f->dot.r = mark.dot;
    f->ndot.r = mark.dot;
    f->mark = mark.mark;
    f->mod = mark.m;
    f->closeok = mark.s1!=Dirty;
    if(mark.s1==Dirty)
        quitok = false;
    if(f->state==Clean && mark.s1==Clean && changes)
        state(f, Dirty);
    else
        state(f, mark.s1);
}

int
undo(void)
{
    File *f;
    int i;
    Mod max;
    if((max = curfile->mod)==0)
        return 0;
    settempfile();
    for(i = 0; i<tempfile.nused; i++){
        f = tempfile.filepptr[i];
        if(f!=cmd && f->mod==max)
            undostep(f);
    }
    return 1;
}

int
readcmd(String *s)
{
    int retcode;

    if(flist == 0)
        (flist = Fopen())->state = Clean;
    addr.r.p1 = 0, addr.r.p2 = flist->nrunes;
    retcode = plan9(flist, '<', s, false);
    Fupdate(flist, false, false);
    flist->mod = 0;
    if (flist->nrunes > BLOCKSIZE)
        error(Etoolong);
    Strzero(&genstr);
    Strinsure(&genstr, flist->nrunes);
    Fchars(flist, genbuf, (Posn)0, flist->nrunes);
    memmove(genstr.s, genbuf, flist->nrunes*RUNESIZE);
    genstr.n = flist->nrunes;
    Straddc(&genstr, '\0');
    return retcode;
}

void
cd(String *str)
{
    int i;
    File *f;
    String *t;

    t = tmpcstr("/bin/pwd");
    Straddc(t, '\0');
    if (flist) {
        Fclose(flist);
        flist = 0;
    }
    if (readcmd(t) != 0) {
        Strduplstr(&genstr, tmprstr(baddir, sizeof(baddir)/sizeof(wchar_t)));
        Straddc(&genstr, '\0');
    }
    freetmpstr(t);
    Strduplstr(&wd, &genstr);
    if(wd.s[0] == 0){
        wd.n = 0;
        warn(Wpwd);
    }else if(wd.s[wd.n-2] == '\n'){
        --wd.n;
        wd.s[wd.n-1]='/';
    }
    if(chdir(getname((File *)0, str, false)? genc : home))
        syserror("chdir");
    settempfile();
    for(i=0; i<tempfile.nused; i++){
        f = tempfile.filepptr[i];
        if(f!=cmd && f->name.s[0]!='/' && f->name.s[0]!=0){
            Strinsert(&f->name, &wd, (Posn)0);
            sortname(f);
        }
    }
}

int
loadflist(String *s)
{
    int c, i;

    c = s->s[0];
    for(i = 0; s->s[i]==' ' || s->s[i]=='\t'; i++)
        ;
    if((c==' ' || c=='\t') && s->s[i]!='\n'){
        if(s->s[i]=='<'){
            Strdelete(s, 0L, (int64_t)i+1);
            readcmd(s);
        }else{
            Strzero(&genstr);
            while((c = s->s[i++]) && c!='\n')
                Straddc(&genstr, c);
            Straddc(&genstr, '\0');
        }
    }else{
        if(c != '\n')
            error(Eblank);
        Strdupl(&genstr, empty);
    }
    if(genc)
        free(genc);
    genc = Strtoc(&genstr);
    return genstr.s[0];
}

File *
readflist(int readall, int delete)
{
    Posn i;
    int c;
    File *f;
    String *t;

    for(i=0,f=0; f==0 || readall || delete; i++){   /* ++ skips blank */
        Strdelete(&genstr, (Posn)0, i);
        for(i=0; (c = genstr.s[i])==' ' || c=='\t' || c=='\n'; i++)
            ;
        if(i >= genstr.n)
            break;
        Strdelete(&genstr, (Posn)0, i);
        for(i=0; (c=genstr.s[i]) && c!=' ' && c!='\t' && c!='\n'; i++)
            ;

        if(i == 0)
            break;
        genstr.s[i] = 0;
        t = tmprstr(genstr.s, i+1);
        f = lookfile(t, false);
        if(delete){
            if(f == 0)
                warn_S(Wfile, t);
            else
                trytoclose(f);
        }else if(f==0 && readall)
            Fsetname(f = newfile(), t);
    }
    return f;
}

File *
tofile(String *s)
{
    File *f = NULL;

    if(s->s[0] != L' ')
        error(Eblank);

    if (loadflist(s) == 0)
        f = lookfile(&genstr, false);

    if (f == NULL)
        f = lookfile(&genstr, true);

    if (f == NULL)
        f = readflist(false, false);

    if (f == NULL)
        error_s(Emenu, genc);

    return current(f);
}

File *
getfile(String *s)
{
    File *f;

    if(loadflist(s) == 0)
        Fsetname(f = newfile(), &genstr);
    else if((f=readflist(true, false)) == 0)
        error(Eblank);
    return current(f);
}

void
closefiles(File *f, String *s)
{
    if(s->s[0] == 0){
        if(f == 0)
            error(Enofile);
        trytoclose(f);
        return;
    }
    if(s->s[0] != L' ')
        error(Eblank);
    if(loadflist(s) == 0)
        error(Enewline);
    readflist(false, true);
}

void
copy(File *f, Address addr2)
{
    Posn p;
    int ni;
    for(p=addr.r.p1; p<addr.r.p2; p+=ni){
        ni = addr.r.p2-p;
        if(ni > BLOCKSIZE)
            ni = BLOCKSIZE;
        Fchars(f, genbuf, p, p+ni);
        Finsert(addr2.f, tmprstr(genbuf, ni), addr2.r.p2);
    }
    addr2.f->ndot.r.p2 = addr2.r.p2+(f->dot.r.p2-f->dot.r.p1);
    addr2.f->ndot.r.p1 = addr2.r.p2;
}

void
move(File *f, Address addr2)
{
    if(addr.r.p2 <= addr2.r.p2){
        Fdelete(f, addr.r.p1, addr.r.p2);
        copy(f, addr2);
    }else if(addr.r.p1 >= addr2.r.p2){
        copy(f, addr2);
        Fdelete(f, addr.r.p1, addr.r.p2);
    }else
        error(Eoverlap);
}

Posn
nlcount(File *f, Posn p0, Posn p1)
{
    Posn nl = 0;

    Fgetcset(f, p0);
    while(p0++<p1)
        if(Fgetc(f)=='\n')
            nl++;
    return nl;
}

void
printposn(File *f, int charsonly)
{
    Posn l1, l2;

    if(!charsonly){
        l1 = 1+nlcount(f, (Posn)0, addr.r.p1);
        l2 = l1+nlcount(f, addr.r.p1, addr.r.p2);
        /* check if addr ends with '\n' */
        if(addr.r.p2>0 && addr.r.p2>addr.r.p1 && (Fgetcset(f, addr.r.p2-1),Fgetc(f)=='\n'))
            --l2;
        dprint(L"%lu", l1);
        if(l2 != l1)
            dprint(L",%lu", l2);
        dprint(L"; ");
    }
    dprint(L"#%lu", addr.r.p1);
    if(addr.r.p2 != addr.r.p1)
        dprint(L",#%lu", addr.r.p2);
    dprint(L"\n");
}

void
settempfile(void)
{
    if(tempfile.nalloc < file.nused){
        free(tempfile.listptr);
        tempfile.listptr = emalloc(sizeof(*tempfile.filepptr) * file.nused);
        tempfile.nalloc = file.nused;
    }
    tempfile.nused = file.nused;
    memmove(&tempfile.filepptr[0], &file.filepptr[0], file.nused*sizeof(File*));
}
