#include <stdio.h>
#include <stdlib.h>
// for lseek
#include <unistd.h>
// for open
#include <fcntl.h>
// for open
#include <sys/stat.h>
// for off_t in lseek
#include <sys/types.h>

int main() {
	char* fname = "ssu_test.txt";
	off_t fsize;
	int fd;

	// ssu_test.txt 파일을 읽기 권한으로 염
	if ((fd = open(fname, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}
	// lseek 의 리턴 값은 성공적으로 이동했을 때 이동하여 위치하게 된 seek position 이 된다.
	// SEEK_END 로부터 0바이트 떨어진 곳으로 이동하라는 명령은 성공하게 된다면
	// 파일의 맨 끝 바이트로 이동하게 되고, 이는 곧 파일의 크기가 된다.
	// seek position 의 이동에 실패하게 된다면 -1 을 리턴한다.
	if ((fsize = lseek(fd, 0, SEEK_END)) < 0) {
		fprintf(stderr, "lseek error\n");
		exit(1);
	}
	printf("The size of <%s> is %ld bytes.\n", fname, fsize);
	exit(0);
}
