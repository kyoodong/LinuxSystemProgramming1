#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//close() 사용 위해 
#include <fcntl.h>//oflag 인자 사용 위해
#include <sys/stat.h>// 모드 상수 사용 위해 
#include <sys/types.h>// mode_t타입 사용 위해

//S_IRUSR: 파일 소유자의 읽기 권한을 지정, 00400 사용해도 됨.
//S_IWUSR: 파일 소유자의 쓰기 권한을 지정, 00200 사용해도 됨.
//S_IRCRP: 그룹 사용자의 읽기 권한을 지정, 00040 사용해도 됨.
//S_IROTH: 다른 사용자의 읽기 권한을 지정, 00004 사용해도 됨. 
#define CREAT_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

char buf1[]="1234567890";
char buf2[]="ABCDEFGHIJ";

int main(void){
    char *fname = "ssu_hole.txt";
    int fd; //file desctiptor
    
    if((fd=creat(fname,CREAT_MODE))<0){	//파일 creat 
	fprintf(stderr, "creat error for %s\n",fname);
	exit(1);
    }
    if(write(fd,buf1,12)!=12){		//buf1내용을 파일에 write, 12byte 사용. 
	fprintf(stderr,"buf1 write error\n");
	exit(1);
    }
    if(lseek(fd,15000,SEEK_SET)<0){	//offset위치를SEEK_SET(처음위치로 지정), 그로부터 15000byte이동
	fprintf(stderr,"lseek error\n");
	exit(1);
    }
    if(write(fd,buf2,12)!=12){	//buf2내용을 파일에 write, 12byte사용. 
	fprintf(stderr,"buf2 write error\n");
	exit(1);
    }
    exit(0);
}

