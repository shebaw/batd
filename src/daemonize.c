#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "daemonize.h"

int daemonize()
{
	int maxfd, fd;
	// change to rootdir not to lock the filesystem we are in from
	// unmounting
	if (chdir("/") != 0) {
		return -1;
	}
	// clear file mode creation mask
	umask(0);

	// close open files
	maxfd = sysconf(_SC_OPEN_MAX);
	if (maxfd == -1) {
		maxfd = 8192;
	}
	for (fd = 0; fd < maxfd; ++fd) {
		close(fd);
	}

	// redirect standard fds to dev/null
	fd = open("/dev/null", O_RDWR);
	if (fd != STDIN_FILENO) {
		return -1;
	}

	if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO ||
		dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
		return -1;
	}

	return 0;
}

