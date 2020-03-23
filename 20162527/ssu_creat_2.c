#include<stdio.h>
#include<stdlib.h>

//creat, open 하기 위해 필요한 헤더들
#include<fcntl.h> //oflag 인자 사용하기 위해
#include<sys/types.h> // mode_t 타입을 사용하기 위해
#include<sys/stat.h> // 생성하고 오픈할 파일의 접근모드 지정 위해

//close 하기 위해 필요한 헤더
#include<unistd.h>

int main(void){
	char *fname = "ssu_test.txt";
	int fd; //파일 디스크립터 선언

	if((fd = creat(fname,0666)) < 0){ //기본모드로 파일 생성
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1);
	}
	else{
		close(fd); //파일 닫음
		fd = open(fname, O_RDWR); //생성했던 그 파일을 읽고 쓰기 모드로 오픈
		printf("Succeeded!\n<%s> is new readable and writable\n", fname);
	}

	exit(0);
}
