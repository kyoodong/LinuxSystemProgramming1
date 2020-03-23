#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CREAT_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

char buf1[] = "1234567890";
char buf2[] = "ABCDEFGHIJ";

int main() {
	char* fname = "ssu_hole.txt";
	int fd;

	// ssu_hole.txt 파일을 0644 권한으로 생성
	if ((fd = creat(fname, CREAT_MODE)) < 0) {
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1);
	}
	// ssu_hole.txt 파일에 buf1 의 12바이트를 출력
	// 즉 1234567890\0\0 을 씀
	if (write(fd, buf1, 12) != 12) {
		fprintf(stderr, "buf1 write error\n");
		exit(1);
	}

	// 파일의 시작 부분에서부터 15000 바이트 떨어진 곳으로 seek position을 이동
	// 이 때 파일의 크기보다 큰 곳으로 이동하게 된다면
	// 파일은 그 크기만큼 늘어나게 되고, 빈 곳은 \0으로 채워진다.
	if (lseek(fd, (off_t) 15000L, SEEK_SET) < 0) {
		fprintf(stderr, "lseek error\n");
		exit(1);
	}

	// 15000바이트 부분부터 buf2의 12바이트를 쓴다.
	// 즉 ABCDEFGHIJ\0\0을 쓰게 된다.
	if (write(fd, buf2, 12) != 12) {
		fprintf(stderr, "buf2 write error\n");
		exit(1);
	}
	exit(0);
}
