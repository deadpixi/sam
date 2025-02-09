/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include "sam.h"
#include "parse.h"

#include <limits.h>

extern  jmp_buf mainloop;
extern  bool forked;

char    errfile[PATH_MAX + 1];
String  plan9cmd;   /* null terminated */
Buffer  *plan9buf;
void    checkerrs(void);

int
plan9(File *f, int type, String *s, int nest)
{
    int64_t l;
    int m;
    int pid, fd;
    int retcode;
    int pipe1[2], pipe2[2];

    if(s->s[0]==0 && plan9cmd.s[0]==0)
        error(Enocmd);
    else if(s->s[0])
        Strduplstr(&plan9cmd, s);
    if(downloaded)
        samerr(errfile);
    else
        strcpy(errfile, "/dev/tty");
    if(type!='!' && pipe(pipe1)==-1)
        error(Epipe);
    if(type=='|')
        snarf(f, addr.r.p1, addr.r.p2, plan9buf, true);
    if(downloaded)
        remove(errfile);
    if((pid=fork()) == 0){
        forked = true;
        if(downloaded){ /* also put nasty fd's into errfile */
            fd = creat(errfile, 0600L);
            if(fd < 0)
                fd = creat("/dev/null", 0600L);
            dup2(fd, 2);
            close(fd);
            /* 2 now points at err file */
            if(type == '>')
                dup2(2, 1);
            else if(type=='!'){
                dup2(2, 1);
                fd = open("/dev/null", 0);
                dup2(fd, 0);
                close(fd);
            }
        }
        if(type != '!') {
            if(type=='<' || type=='|')
                dup2(pipe1[1], 1);
            else if(type == '>')
                dup2(pipe1[0], 0);
            close(pipe1[0]);
            close(pipe1[1]);
        }
        if(type == '|'){
            if(pipe(pipe2) == -1)
                exit(EXIT_FAILURE);
            if((pid = fork())==0){
                /*
                 * It's ok if we get SIGPIPE here
                 */
                close(pipe2[0]);
                io = fdopen(pipe2[1], "w");
                if ((retcode = !setjmp(mainloop))){  /* assignment = */
                    char *c;
                    for(l = 0; l<plan9buf->nrunes; l+=m){
                        m = plan9buf->nrunes-l;
                        if(m>BLOCKSIZE-1)
                            m = BLOCKSIZE-1;
                        Bread(plan9buf, genbuf, m, l);
                        genbuf[m] = 0;
                        c = Strtoc(tmprstr(genbuf, m+1));
                        Write(io, c, strlen(c));
                        free(c);
                    }
                }
                exit(retcode? EXIT_FAILURE : EXIT_SUCCESS);
            }
            if(pid==-1){
                fprintf(stderr, "Can't fork?!\n");
                exit(EXIT_FAILURE);
            }
            dup2(pipe2[0], 0);
            close(pipe2[0]);
            close(pipe2[1]);
        }
        if(type=='<'){
            close(0);   /* so it won't read from terminal */
            open("/dev/null", 0);
        }
        execl(shpath, sh, "-c", Strtoc(&plan9cmd), NULL);
        exit(EXIT_FAILURE);
    }
    if(pid == -1)
        error(Efork);
    if(type=='<' || type=='|'){
        bool nulls;
        if(downloaded && addr.r.p1 != addr.r.p2)
            outTl(Hsnarflen, addr.r.p2-addr.r.p1);
        snarf(f, addr.r.p1, addr.r.p2, snarfbuf, false);
        Fdelete(f, addr.r.p1, addr.r.p2);
        close(pipe1[1]);
        io = fdopen(pipe1[0], "r");
        f->tdot.p1 = -1;
        f->ndot.r.p2 = addr.r.p2+readio(f, &nulls, 0);
        f->ndot.r.p1 = addr.r.p2;
        closeio((Posn)-1);
    }else if(type=='>'){
        close(pipe1[0]);
        io = fdopen(pipe1[1], "w");
        bpipeok = true;
        writeio(f);
        bpipeok = false;
        closeio((Posn)-1);
    }
    retcode = waitfor(pid);
    if(type=='|' || type=='<')
        if(retcode!=0)
            warn(Wbadstatus);
    if(downloaded)
        checkerrs();
    if(!nest)
        dprint(L"!\n");
    return retcode;
}

void
checkerrs(void)
{
    char buf[256];
    int f, n, nl;
    char *p;
    int64_t l;

    if(statfile(errfile, 0, 0, 0, &l, 0) > 0 && l != 0){
        if((f=open((char *)errfile, 0)) != -1){
            if((n=read(f, buf, sizeof buf-1)) > 0){
                for(nl=0,p=buf; nl<3 && p<&buf[n]; p++)
                    if(*p=='\n')
                        nl++;
                *p = 0;
                dprint(L"%s", buf);
                if(p-buf < l-1)
                    dprint(L"(sam: more in %s)\n", errfile);
            }
            close(f);
        }
    }else
        remove((char *)errfile);
}
