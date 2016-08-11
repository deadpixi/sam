/* Copyright 2013-2015 Rob King <jking@deadpixi.com>
 * This file may be freely redistributed in source or binary form with or without modification.
 * No warranty is expressed or implied; use at your own risk.
 */

#define _POSIX_C_SOURCE 200112L
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../config.h"

#define PARENT_READ	readpipe[0]
#define CHILD_WRITE	readpipe[1]
#define CHILD_READ	writepipe[0]
#define PARENT_WRITE	writepipe[1]
#define MAX(x, y)	((x) > (y) ? (x) : (y))

char *fifopath = NULL;

void
cleanup(void)
{
	if (fifopath)
	{
		unlink(fifopath);
		free(fifopath);
	}
}

int
main(int argc, char **argv)
{
	const char *home         = getenv("HOME") ? getenv("HOME") : TMPDIR;
	long        pathmax      = pathconf(home, _PC_PATH_MAX) != -1 ? pathconf(home, _PC_PATH_MAX) : PATH_MAX;
	int         writepipe[2] = {-1};
	int         readpipe[2]  = {-1};

	fifopath = calloc(pathmax, sizeof(char));
	if (fifopath == NULL)
	{
		perror("fifopath");
		return EXIT_FAILURE;
	}

	if (pipe(writepipe) != 0 || pipe(readpipe) != 0)
	{
		perror("pipe");
		return EXIT_FAILURE;
	}

	snprintf(fifopath, pathmax, "%s/.sam.fifo", home);
	unlink(fifopath);
	if (mkfifo(fifopath, 0600) != 0)
	{
		perror("mkfifo");
		return EXIT_FAILURE;
	}

	fifopath = fifopath;
	atexit(cleanup);

	int fifofd = open(fifopath, O_RDWR);
	if (fifofd < 0)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	pid_t child = fork();
	if (child == 0)
	{
		close(PARENT_WRITE);
		close(PARENT_READ);

		dup2(CHILD_READ,  STDIN_FILENO);  close(CHILD_READ);
		dup2(CHILD_WRITE, STDOUT_FILENO); close(CHILD_WRITE);

		execlp("sam", "sam", "-R", NULL);
		return EXIT_FAILURE;
	}
	else if (child < 0)
	{
		perror("fork");
		return EXIT_FAILURE;
	}

	close(CHILD_READ);
	close(CHILD_WRITE);

	fd_set readfds;
	fd_set writefds;

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	FD_SET(fifofd, &readfds);
	FD_SET(PARENT_READ, &readfds);

	while (select(MAX(STDIN_FILENO, MAX(PARENT_READ, fifofd)) + 1, &readfds, NULL, NULL, NULL) >= 0)
	{
		ssize_t count = 0;
		char	buf[8192];

		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			count = read(STDIN_FILENO, buf, 8192);
			if (count <= 0)
			{
				exit(EXIT_SUCCESS);
			}
			write(PARENT_WRITE, buf, count);
		}

		if (FD_ISSET(fifofd, &readfds))
		{
			memset(buf, 0, 256);
			count = read(fifofd, buf, 253);
			if (count <= 0)
			{
				exit(EXIT_SUCCESS);
			}
			write(STDOUT_FILENO, "\x19\xff\x00", 3);
			write(STDOUT_FILENO, buf, 255);
		}

		if (FD_ISSET(PARENT_READ, &readfds))
		{
			count = read(PARENT_READ, buf, 8192);
			if (count <= 0)
			{
                exit(EXIT_SUCCESS);
            }
			write(STDOUT_FILENO, buf, count);
		}

		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(fifofd, &readfds);
		FD_SET(PARENT_READ, &readfds);
	}

	return EXIT_SUCCESS;
}
