#include <stdio.h>
#include <stdlib.h>

//open()을 위한 헤더들
#include <fcntl.h> //oflag 인자를 사용하기 위해 선언
#include <sys/types.h> //mode의 자료형을 사용하기 위해 선언
#include <sys/stat.h> //오픈할 파일의 접근 권한을 부여하기 위해 선언

//close()를 위한 헤더
#include <unistd.h>

int main(void)
{	
	char *fname = "ssu_test.txt"; //접근할 파일 이름 저장
	int fd; //파일 디스크립터 생성

	//file을 읽기 모드로 오픈한다.
	if((fd = open(fname, O_RDONLY)) < 0){ //오픈이 실패했을 경우
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}
	else{ //오픈이 성공했을 경우
		printf("Success!\nFilename : %s\nDescriptor : %d\n", fname, fd);
		close(fd);
	}
	exit(0);
}

