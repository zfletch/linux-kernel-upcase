#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

int main (int argc, char** argv)
{
	if (argv[1] == NULL) {
		printf("%s: <string>\n", argv[0]);
		exit(0);
	}

	// open device
	int fd = open("/dev/upcase", O_RDWR);

	pid_t pid = fork();
	printf("%d\n", pid);

	if (pid == 0) {
		while (true) {
			// send argv[1] to upcase
			write(fd, argv[1], strlen(argv[1]));
		}
	} else {
		while (true) {
			// read from upcase and write the result to argv[1]
			read(fd, argv[1], strlen(argv[1]));
			printf("%s\n", argv[1]);
		}
	}

	close(fd);

	return 0;
}
