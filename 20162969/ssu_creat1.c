//creat()예제1(p.68)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//close() 사용 위해 
#include <fcntl.h>//creat()위해 반드시 넣을 것. oflag 사용 위해 
#include <sys/types.h>//mode_t 타입 사용 위해 
#include <sys/stat.h>//creat하는 파일의 접근 권한 부여 위해

int main(void)
{
    char *fname = "ssu_test.txt";//파일 이름 
    int fd;//file descriptor 

    if((fd = creat(fname, 0666))<0){  //매크로명(기본모드)으로 명확하게 쓰는게 좋다
	fprintf(stderr,  "creat error for %s\n", fname);
	exit(1);
    }
    else{
	printf("Success!\nFilename:%s\nDescriptor:%d\n",fname,fd);
	close(fd);
    }
    exit(0);
}

