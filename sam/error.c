/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include "sam.h"

static char *emsg[]={
    /* error_s */
    "can't open",
    "can't create",
    "not in menu:",
    "changes to",
    "I/O error:",
    /* error_c */
    "unknown command",
    "no operand for",
    "bad delimiter",
    /* error */
    "can't fork",
    "interrupt",
    "address",
    "search",
    "pattern",
    "newline expected",
    "blank expected",
    "pattern expected",
    "can't nest X or Y",
    "unmatched `}'",
    "command takes no address",
    "addresses overlap",
    "substitution",
    "& match too long",
    "bad \\ in rhs",
    "address range",
    "changes not in sequence",
    "addresses out of order",
    "no file name",
    "unmatched `('",
    "unmatched `)'",
    "malformed `[]'",
    "malformed regexp",
    "reg. exp. list overflow",
    "plan 9 command",
    "can't pipe",
    "no current file",
    "string too long",
    "changed files",
    "empty string",
    "file search",
    "non-unique match for \"\"",
    "tag match too long",
    "too many subexpressions",
    "temporary file too large",
    "file is append-only",
};
static char *wmsg[]={
    /* warn_s */
    "duplicate file name",
    "no such file",
    "write might change good version of",
    /* warn_S */
    "files might be aliased",
    /* warn */
    "null characters elided; file will be different upon write",
    "can't run pwd",
    "last char not newline",
    "exit status not 0",
    "file is not text"
};

void
error(Err s)
{
    char buf[512];

    snprintf(buf, sizeof(buf) - 1, "?%s", emsg[s]);
    hiccough(buf);
}

void
error_s(Err s, char *a)
{
    char buf[512];

    snprintf(buf, sizeof(buf) - 1, "?%s \"%s\"", emsg[s], a);
    hiccough(buf);
}

void
error_c(Err s, int c)
{
    char buf[512];

    snprintf(buf, sizeof(buf) - 1, "?%s `%c'", emsg[s], c);
    hiccough(buf);
}

void
warn(Warn s)
{
    dprint(L"?warning: %s\n", wmsg[s]);
}

void
warn_S(Warn s, String *a)
{
    print_s(wmsg[s], a);
}

void
warn_SS(Warn s, String *a, String *b)
{
    print_ss(wmsg[s], a, b);
}

void
warn_s(Warn s, char *a)
{
    dprint(L"?warning: %s `%s'\n", wmsg[s], a);
}

void
termwrite(wchar_t *p)
{
    size_t l = wcslen(p);

    if(downloaded){
        if(cmd)
            Finsert(cmd, tmprstr(p, l), cmdpt);
        else
            Strinsert(&cmdstr, tmprstr(p, l), cmdstr.n);
        cmdptadv += wcslen(p);
    }else
        fprintf(stderr, "%ls", p);
}

