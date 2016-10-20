/* Copyright 2013-2015 Rob King <jking@deadpixi.com>
 * This file may be freely redistributed in source or binary form with or without modification.
 * No warranty is expressed or implied; use at your own risk.
 */

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PARENT_READ  readpipe[0]
#define CHILD_WRITE  readpipe[1]
#define CHILD_READ   writepipe[0]
#define PARENT_WRITE writepipe[1]
#define MAX(x, y)    ((x) > (y) ? (x) : (y))
#define CHECKEDWRITE(f, b, c) if (write(f, b, c) != c) exit(EXIT_FAILURE)

char path[PATH_MAX + 1];

void
cleanup(void)
{
    unlink(path);
}

int
main(int argc, char **argv)
{
    int fifo = -1;
    int nfd = 0;
    int writepipe[2] = {-1};
    int readpipe[2]  = {-1};
    struct passwd *pwent = NULL;
    pid_t child = -1;
    fd_set rfds;

    pwent = getpwuid(getuid());
    if (!pwent || !pwent->pw_dir || !pwent->pw_dir[0])
        return perror("pwent"), EXIT_FAILURE;

    strncpy(path, pwent->pw_dir, PATH_MAX);
    strncat(path, "/.sam.fifo", PATH_MAX);

    if (pipe(writepipe) != 0 || pipe(readpipe) != 0)
        return perror("pipe"), EXIT_FAILURE;

    unlink(path);
    if (mkfifo(path, 0600) != 0)
        return perror("mkfifo"), EXIT_FAILURE;

    atexit(cleanup);

    fifo = open(path, O_RDWR);
    if (fifo < 0)
        return perror("open"), EXIT_FAILURE;

    child = fork();
    if (child == 0){
        close(PARENT_WRITE);
        close(PARENT_READ);

        dup2(CHILD_READ,  STDIN_FILENO);  close(CHILD_READ);
        dup2(CHILD_WRITE, STDOUT_FILENO); close(CHILD_WRITE);

        execlp("sam", "sam", "-R", NULL);
        return EXIT_FAILURE;
    } else if (child < 0){
        perror("fork");
        return EXIT_FAILURE;
    }

    close(CHILD_READ);
    close(CHILD_WRITE);

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_SET(fifo, &rfds);
    FD_SET(PARENT_READ, &rfds);

    nfd = MAX(STDIN_FILENO, MAX(PARENT_READ, fifo)) + 1;
    while (select(nfd, &rfds, NULL, NULL, NULL) >= 0){
        ssize_t count = 0;
        char buf[8192];

        if (FD_ISSET(STDIN_FILENO, &rfds)){
            count = read(STDIN_FILENO, buf, 8192);
            if (count <= 0)
                exit(EXIT_SUCCESS);
            CHECKEDWRITE(PARENT_WRITE, buf, count);
        }

        if (FD_ISSET(fifo, &rfds)){
            memset(buf, 0, 256);
            count = read(fifo, buf, 253);
            if (count <= 0)
                exit(EXIT_SUCCESS);
            CHECKEDWRITE(STDOUT_FILENO, "\x19\xff\x00", 3);
            CHECKEDWRITE(STDOUT_FILENO, buf, 255);
        }

        if (FD_ISSET(PARENT_READ, &rfds)){
            count = read(PARENT_READ, buf, 8192);
            if (count <= 0)
                exit(EXIT_SUCCESS);
            CHECKEDWRITE(STDOUT_FILENO, buf, count);
        }

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(fifo, &rfds);
        FD_SET(PARENT_READ, &rfds);
    }

    return EXIT_SUCCESS;
}
