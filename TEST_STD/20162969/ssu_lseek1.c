//lseek()예제 1(p.74)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//close()사용 위해
#include <fcntl.h>//oflag인자 사용 위해 
#include <sys/types.h>//mode_t자료형과 off_t자료형 사용 위해 
#include <sys/stat.h> //open할 파일 접근모드 지정 위해 

int main(void)
{
    char *fname = "ssu_test.txt";
    off_t fsize;//파일의 크기 확인 방법 
    int fd;//file directory 

    //에러 처리 
    if((fd=open(fname,O_RDONLY))<0){//==-1로 써도 무방. O_RDONLY(읽기전용)모드로 open 
	fprintf(stderr, "open error for %s\n",fname);
	exit(1);//실패시 
    }
    if((fsize = lseek(fd,(off_t)0,SEEK_END))<0){//파일크기확인법(SEEK_END로 offset위치가 맨끝으로=사이즈),캐스팅할것 
	fprintf(stderr,"lseek error\n");
	exit(1);//실패시 
    }
    printf("The size of <%s> is %ldbytes.\n",fname, fsize);

    exit(0);//성공시
}

