#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// creat 함수를 위함
#include <fcntl.h>
// mode_t
#include <sys/types.h>
// mode
#include <sys/stat.h>

int main(void) {
	char *fname = "ssu_test.txt";
	int fd;

	// fname 파일을 0666 권한으로 생성함
	// 단 파일이 이미 존재하는 경우 이 파일을 재작성함
	// 재작성이므로 기존의 내용은 다 지워짐 (하지만 권한까지 재설정하진 않음)
	// 0666에서 0은 8진수를 의미하며 각 자리 6은 왼쪽부터 소유자, 그룹, 기타의 권한을 말하며
	// 하나의 수는 1,2,4 각 비트에 의미를 두어 1비트는 액세스 권한, 2비트는 쓰기 권한, 3비트는 읽기권한으로
	// 모든 권한을 다 부여하게되면 7이 된다.
	if ((fd = creat(fname, 0666)) < 0) {
		fprintf(stderr, "create error for %s\n", fname);
		exit(1);
	}
	else {
		printf("Success!\nFilename : %s\nDescriptor : %d\n", fname, fd);
		close(fd);
	}
	exit(0);
}
