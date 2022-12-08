#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <selinux/selinux.h>
#include <selinux/context.h>

int main(int argc, char **argv)
{
	int rc;
	const char *context_s;
	char *context_tmp;
	context_t context;

	if (argc != 2) {
		fprintf(stderr, "usage:  %s newdomain\n", argv[0]);
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

	rc = setcon(context_s);
	if (rc < 0) {
		perror("setcon failed");
		exit(-1);
	}

	printf("All systems go\n");
	exit(0);
}

