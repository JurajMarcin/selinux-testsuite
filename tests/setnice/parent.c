#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <selinux/selinux.h>
#include <selinux/context.h>


int main(int argc, char **argv)
{
	char buf[1];
	int pid, rc, rc2, fd[2], fd2[2];
	const char *context_s;
	char *context_tmp;
	context_t context;

	if (argc != 3) {
		fprintf(stderr, "usage:  %s newdomain program\n", argv[0]);
		exit(-1);
	}

	rc = getcon(&context_tmp);
	if (rc < 0) {
		fprintf(stderr, "%s:  unable to get my context\n", argv[0]);
		exit(-1);

	}

	context = context_new(context_tmp);
	if (!context) {
		fprintf(stderr, "%s:  unable to create context structure\n", argv[0]);
		exit(-1);
	}

	if (context_type_set(context, argv[1])) {
		fprintf(stderr, "%s:  unable to set new type\n", argv[0]);
		exit(-1);
	}

	freecon(context_tmp);
	context_s = context_str(context);
	if (!context_s) {
		fprintf(stderr, "%s:  unable to obtain new context string\n", argv[0]);
		exit(-1);
	}

	rc = setexeccon(context_s);
	if (rc < 0) {
		fprintf(stderr, "%s:  unable to set exec context to %s\n", argv[0], context_s);
		exit(-1);
	}

	rc = pipe(fd);
	if (rc < 0) {
		perror("pipe");
		exit(-1);
	}

	rc = pipe(fd2);
	if (rc < 0) {
		perror("pipe");
		exit(-1);
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(-1);
	} else if (pid == 0) {
		dup2(fd[0], 0);
		dup2(fd2[1], 1);
		execv(argv[2], argv + 2);
		buf[0] = -1;
		write(1, buf, sizeof(buf));
		perror(argv[2]);
		exit(-1);
	}

	rc = read(fd2[0], buf, sizeof(buf));
	if (rc < 0) {
		perror("read");
		exit(-1);
	}

	if (buf[0]) {
		fprintf(stderr, "%s:  child died unexpectedly\n", argv[0]);
		exit(-1);
	}


	rc =  setpriority(0, pid, 10);
	rc2 = write(fd[1], buf, sizeof(buf));

	if (rc2 < 0) {
		perror("write");
		exit(-1);
	}
	if (rc < 0) {
		perror("setnice: setnice(pid,pid)");
		exit(1);
	}
	exit(0);
}
