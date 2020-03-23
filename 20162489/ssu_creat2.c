#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void) {
	char* fname = "ssu_test.txt";
	int fd;

	// ssu_test.txt 파일을 0666 권한으로 생성
	if ((fd = creat(fname, 0666)) < 0) {
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1);
	}
	else {
		// ssu_test.txt 파일을 열었던 것을 닫고 해당 파일을 읽기권한으로 다시 염
		close(fd);
		fd = open(fname, O_RDWR);

		// 사실 open으로 파일을 또 열었기에 fd 가 4라고 생각할 수도 있지만
		// close 에서는 fd를 os에 반환하였으므로 3번부터 다시 사용할 수 있기에
		// 새로 열린 ssu_test.txt 또한 fd 가 3임. (절대 같은 파일을 또 열어서 그런것이 아님)
		printf("Succeeded!\n<%s> is new readable and writable\n", fname);
	}
	exit(0);
}

