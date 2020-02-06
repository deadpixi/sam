/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#include "sam.h"

#define NSYSFILE    3
#define NOFILE      128

#define MIN(x, y) ((x) < (y)? (x) : (y))

void
checkqid(File *f)
{
    int i, w;
    File *g;

    w = whichmenu(f);
    for(i=1; i<file.nused; i++){
        g = file.filepptr[i];
        if(w == i)
            continue;
        if(f->dev==g->dev && f->qid==g->qid)
            warn_SS(Wdupfile, &f->name, &g->name);
    }
}

void
writef(File *f)
{
    wchar_t c;
    Posn n;
    char *name;
    int i, samename, newfile;
    uint64_t dev, qid;
    int64_t mtime, appendonly, length;

    newfile = 0;
    samename = Strcmp(&genstr, &f->name) == 0;
    name = Strtoc(&f->name);
    i = statfile(name, &dev, &qid, &mtime, 0, 0);
    if(i == -1)
        newfile++;
    else if(samename &&
            (f->dev!=dev || f->qid!=qid || f->date<mtime)){
        f->dev = dev;
        f->qid = qid;
        f->date = mtime;
        warn_S(Wdate, &genstr);
        return;
    }
    if(genc)
        free(genc);
    genc = Strtoc(&genstr);
    if ((io = fopen(genc, "w+")) == NULL)
        error_s(Ecreate, genc);
    dprint(L"%s: ", genc);
    if(statfd(fileno(io), 0, 0, 0, &length, &appendonly) > 0 && appendonly && length>0)
        error(Eappend);
    n = writeio(f);
    if(f->name.s[0]==0 || samename)
        state(f, addr.r.p1==0 && addr.r.p2==f->nrunes? Clean : Dirty);
    if(newfile)
        dprint(L"(new file) ");
    if(addr.r.p2>0 && Fchars(f, &c, addr.r.p2-1, addr.r.p2) && c!='\n')
        warn(Wnotnewline);
    closeio(n);
    if(f->name.s[0]==0 || samename){
        if(statfile(name, &dev, &qid, &mtime, 0, 0) > 0){
            f->dev = dev;
            f->qid = qid;
            f->date = mtime;
            checkqid(f);
        }
    }
}

Posn
readio(File *f, bool *nulls, bool setdate)
{
    uint64_t dev, qid;
    int64_t mtime;
    int olderr = errno;
    Posn nt = 0;

    wchar_t buf[2] = {0};
    while (true){
        buf[0] = fgetwc(io);
        if (buf[0] == WEOF){
            if (errno == EILSEQ){
                clearerr(io);
                fflush(io);
                fgetc(io); /* POSIX does not mandate that we advance here... */
                buf[0] = UNICODE_REPLACEMENT_CHAR;
                errno = 0;
            } else
                break;
        }

        if (buf[0] == 0)
            *nulls = true;

        nt++;
        Finsert(f, tmprstr(buf, 1), addr.r.p2);
    }

    if (setdate){
        if (statfd(fileno(io), &dev, &qid, &mtime, 0, 0) > 0){
            f->dev = dev;
            f->qid = qid;
            f->date = mtime;
            checkqid(f);
        }
    }

    errno = olderr;
    return nt;
}

void
flushio(void)
{
    if (io)
        fflush(io);
}

Posn
writeio(File *f)
{
    int m, n;
    Posn p = addr.r.p1;
    char *c;

    while(p < addr.r.p2){
        if(addr.r.p2-p>BLOCKSIZE)
            n = BLOCKSIZE;
        else
            n = addr.r.p2-p;
        if(Fchars(f, genbuf, p, p+n)!=n)
            panic("writef read");
        c = Strtoc(tmprstr(genbuf, n));
        m = strlen(c);
        if (m < n)
            panic("corrupted file");
        if(Write(io, c, m) != m){
            free(c);
            if(p > 0)
                p += n;
            break;
        }
        free(c);
        p += n;
    }
    return p-addr.r.p1;
}
void
closeio(Posn p)
{
    fclose(io);
    io = NULL;
    if(p >= 0)
        dprint(L"#%lu\n", p);
}

char exname[PATH_MAX + 1];
int remotefd0 = 0;
int remotefd1 = 1;
int exfd = -1;

void
bootterm(char *machine)
{
    char fd[100];
    int ph2t[2], pt2h[2];

    snprintf(fd, sizeof(fd) - 1, "%d", exfd);

    if (machine){
        dup2(remotefd0, 0);
        dup2(remotefd1, 1);
        close(remotefd0);
        close(remotefd1);
        if (exfd >= 0)
            execlp(samterm, samterm, "-r", machine, "-f", fd, "-n", exname, NULL);
        else
            execlp(samterm, samterm, "-r", machine, NULL);
        perror("couldn't exec samterm");
        exit(EXIT_FAILURE);
    }

    if (pipe(ph2t)==-1 || pipe(pt2h)==-1)
        panic("pipe");

    machine = machine? machine : "localhost";
    switch (fork()){
        case 0:
            dup2(ph2t[0], 0);
            dup2(pt2h[1], 1);
            close(ph2t[0]);
            close(ph2t[1]);
            close(pt2h[0]);
            close(pt2h[1]);
            if (exfd >= 0)
                execlp(samterm, samterm, "-r", machine, "-f", fd, "-n", exname, NULL);
            else
                execlp(samterm, samterm, "-r", machine, NULL);
            perror("couldn't exec samterm");
            exit(EXIT_FAILURE);
            break;

        case -1:
            panic("can't fork samterm");
            break;
    }

    dup2(pt2h[0], 0);
    dup2(ph2t[1], 1);
    close(ph2t[0]);
    close(ph2t[1]);
    close(pt2h[0]);
    close(pt2h[1]);
}

void
connectto(char *machine)
{
    int p1[2], p2[2];
    char sockname[FILENAME_MAX + 1] = {0};
    char rarg[FILENAME_MAX + 1] = {0};

    snprintf(sockname, FILENAME_MAX, "%s/sam.remote.%s",
             getenv("RSAMSOCKETPATH")? getenv("RSAMSOCKETPATH") : "/tmp",
             getenv("USER")? getenv("USER") : getenv("LOGNAME")? getenv("LOGNAME") : "nemo");

    snprintf(rarg, FILENAME_MAX, "%s:%s", sockname, exname);

    if(pipe(p1)<0 || pipe(p2)<0){
        dprint(L"can't pipe\n");
        exit(EXIT_FAILURE);
    }
    remotefd0 = p1[0];
    remotefd1 = p2[1];
    switch(fork()){
    case 0:
        dup2(p2[0], 0);
        dup2(p1[1], 1);
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);
        execlp(getenv("RSH") ? getenv("RSH") : RXPATH,
               getenv("RSH") ? getenv("RSH") : RXPATH,
               "-R", rarg,
               machine, rsamname, "-R", sockname,
               NULL);
        dprint(L"can't exec %s\n", RXPATH);
        exit(EXIT_FAILURE);

    case -1:
        dprint(L"can't fork\n");
        exit(EXIT_FAILURE);
    }
    close(p1[1]);
    close(p2[0]);
}

void
removesocket(void)
{
    close(exfd);
    unlink(exname);
    exname[0] = 0;
}

bool
canlocksocket(const char *machine)
{
    int fd = -1;
    const char *path = getenv("SAMSOCKPATH")? getenv("SAMSOCKPATH") : getenv("HOME");
    char lockpath[FILENAME_MAX + 1] = {0};

    if (!path){
        fputs("could not determine command socket path\n", stderr);
        return true;
    }

    snprintf(lockpath, PATH_MAX, "%s/.sam.%s.lock", path, machine? machine : "localhost");
    fd = open(lockpath, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
        return false;

    if (lockf(fd, F_TLOCK, 0) != 0)
        return close(fd), false;

    return true;
}

void
opensocket(const char *machine)
{
    struct sockaddr_un un = {0};
    const char *path = getenv("SAMSOCKPATH")? getenv("SAMSOCKPATH") : getenv("HOME");

    if (!canlocksocket(machine)){
        fputs("could not lock socket\n", stderr);
        return;
    }

    if (!path){
        fputs("could not determine command socket path\n", stderr);
        return;
    }

    snprintf(exname, PATH_MAX, "%s/.sam.%s", path, machine? machine : "localhost");

    if (strlen(exname) >= sizeof(un.sun_path) - 1){
        fputs("command socket path too long\n", stderr);
        return;
    }

    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, exname, sizeof(un.sun_path) - 1);
    if ((exfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0
    ||  bind(exfd, (struct sockaddr *)&un, sizeof(un)) < 0
    ||  listen(exfd, 10) < 0){
        perror("could not open command socket");
        exfd = -1;
        return;
    }

    atexit(removesocket);
}

void
startup(char *machine, bool rflag, bool trylock)
{
    if (!rflag && trylock)
        opensocket(machine);

    if (machine)
        connectto(machine);

    if (!rflag)
        bootterm(machine);

    downloaded = true;
    outTs(Hversion, VERSION);
}
