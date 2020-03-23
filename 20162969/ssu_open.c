#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//close() 사용을 위해 
#include <fcntl.h>//oflag 사용을 위해 
#include <sys/types.h>//mode의 자료형 사용 위해 
#include <sys/stat.h>//open파일 접근권한 부여 위해 

int main(void)
{
  char *fname = "ssu_test.txt";//접근 파일 이름  
  int fd;//file descriptor 생성 

//에러 처리하는 구문 
  if((fd=open(fname, O_RDONLY))<0){    //==-1로 써도 무방. 파일이름,flag종류)
      fprintf(stderr, "open error for %s\n", fname);
      exit(1);	//실패시 
  }
  else
      printf("Success!\nFilename: %s\nDescriptor : %d\n", fname, fd);

  exit(0);	//성공시 
}

