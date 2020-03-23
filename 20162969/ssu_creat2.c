//creat()예제2(p.69)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//close()사용 위해 
#include <fcntl.h>//oflag인자 사용 위해
#include<sys/types.h>//mode_t 타입 사용 위해
#include<sys/stat.h>//creat와 open파일 접근권한부여 위해 

int main(void)
{
    char *fname = "ssu_test.txt";
    int fd;//file descriptor 

    if((fd=creat(fname,0666))<0){//기본모드0666로 파일 creat
	fprintf(stderr, "creat error for %s\n",fname);
	exit(1);
    }
    else{
	close(fd);
	fd=open(fname,O_RDWR);//creat한 파일을 O_RDWR(읽고 쓰기 모드)로 open 
	printf("Succeeded!\n<%s> is new readable and writable\n",fname);
    }
    exit(0);
}

