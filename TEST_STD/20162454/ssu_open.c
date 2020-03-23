#include <stdio.h> //입출력을 위한 표준 라이브러리 
#include <stdlib.h> //프로세스 종료를 위한 표준 라이브러리
#include <unistd.h> //파일 디스크립터를 위한 상수 사용시(0, 1, 2)
#include <fcntl.h> //파일의 용도를 결정하는 oflag 인자(상수) 정의
#include <sys/types.h> //파일 모드의 데이터 타입(_t) 정의
#include <sys/stat.h> //파일의 접근 모드를 결정하는 mode 인자 정의


int main(void)
{
    char *fname = "ssu_test.txt"; //파일 이름
    int fd; //파일 오픈 성공 시 반환될 파일 디스크립터

    if((fd = open(fname, O_RDONLY)) < 0) { //해당 파일을 읽기 전용으로 열고
        fprintf(stderr, "open error for %s\n", fname); //실패할 경우 에러 메시지 출력 후
        exit(1); //에러로 인한 프로세스 종료
    }
    else //파일 오픈 성공할 경우
        printf("Success!\nFilename : %s\nDescriptor : %d\n", fname, fd);

    exit(0); //정상 종료
}
