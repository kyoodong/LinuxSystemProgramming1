#include<stdio.h>
#include<stdlib.h>

//creat 쓰기 위해 선언한 헤더
#include<fcntl.h> //oflag 인자 사용하기 위해
#include<sys/types.h> //mode_t 타입 사용하기 위해
#include<sys/stat.h> //모드 상수 사용하기 위해

//close 쓰기 위해 선언하는 헤더
#include<unistd.h>

//권한을 하나로 묶어서 표현한다.
//파일 소유자에게 일기와 쓰기 권한을 지정하고,
//그룹 사용자와 다른 사용자에게는 읽기 권한만들 부여한다.
#define CREAT_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

char buf1[] = "1234567890";
char buf2[] = "ABCDEFGHIJ";

int main(void){
	char *fname = "ssu_hole.txt";
	int fd; //파일 디스크립터 선언

	if((fd = creat(fname, CREAT_MODE)) < 0) { //파일을 생성한다.
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1);
	}

	if(write(fd, buf1, 12) != 12) { //buf1의 내용을 파일에 쓰는데, 12바이트를 쓰겠다.
		fprintf(stderr, "buf1 write error\n");
		exit(1);
	}
	if(lseek(fd, 15000, SEEK_SET) < 0) { //처음 시작부분부터 15000바이트 이동
		fprintf(stderr, "lseek error\n");
		exit(1);
	}
	if(write(fd, buf2, 12) != 12) { //buf2의 내용을 파일에 쓰는데, 12바이트를 쓰겠다.
		fprintf(stderr, "buf2 write error\n");
		exit(1);
	}

	exit(0);

}
