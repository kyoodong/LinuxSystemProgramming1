#include<stdio.h>
#include<stdlib.h>

//open 사용하기 위한 헤더
#include<fcntl.h> //oflag 인자 사용하기 위해
#include<sys/types.h> //mode_t와 off_t 자료형 사용하기 위해
#include<sys/stat.h> //오픈할 파일의 접근모드 지정하기 위해

//close 사용하기 위한 헤더
#include<unistd.h>

int main(void){
	char *fname = "ssu_test.txt";
	off_t fsize; //offset을 저장하는 타입으로, 파일 사이즈를 측정하기 위해 선언
	int fd; //파일 디스크립터 선언

	if((fd = open(fname, O_RDONLY)) < 0){ //파일을 읽기 전용으로 연다
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

	//파일의 offset 위치를 맨 끝으로 위치하여 리턴되는 값이 파일의 사이즈
	if((fsize = lseek(fd, 0, SEEK_END)) < 0) {
		fprintf(stderr, "lseek error\n");
		exit(1);
	}

	printf("The size of <%s> is %ld bytes.\n", fname, fsize);

	exit(0);
}
