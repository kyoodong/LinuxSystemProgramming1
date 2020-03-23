#include<stdio.h>
#include<stdlib.h>

//creat()위한 헤더 선언
#include<fcntl.h>//oflag인자로 사용하기 위해
#include<sys/types.h>//mode_t 타입을 사용하기 위해
#include<sys/stat.h>//생성할 파일의 접근 권한 부여하기 위해

//close()위한 헤더 선언
#include<unistd.h>

int main(void){
	char *fname = "ssu_test.txt";
	int fd;//파일 디스크립터 선언

	if((fd = creat(fname, 0666)) < 0){ //파일을 기본모드로 생성하겠다.
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1);
	}
	else{
		printf("Success!\nFilename : %s\nDescriptor : %d\n", fname, fd);
		close(fd);
	}

	exit(0);
}
