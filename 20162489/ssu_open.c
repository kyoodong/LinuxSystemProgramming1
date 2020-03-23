#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// file control for 'open'
#include <fcntl.h>
// for mode_t in function 'open'
#include <sys/types.h>
// for mode in function 'open'
#include <sys/stat.h>

int main() {
	char* fname = "ssu_test.txt";
	int fd;

	// ssu_test.txt 파일을 읽기 권한으로 읽고 그 file descriptor 를 리턴받는다.
	// fd 가 -1이면 파일을 여는데 실패한 것이다.
	if ((fd = open(fname, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}
	else printf("Success!\nFilename : %s\nDescriptor : %d\n", fname, fd);
	exit(0);
}
