#include <stdio.h>
#include <string.h>
#include <fcntl.h>

int main (int argc, char** argv)
{
	int fd = open("/dev/upcase", O_RDWR);
	write(fd, "abc", strlen("abc"));

	return 0;
}
