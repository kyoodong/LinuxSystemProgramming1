#include <stdio.h> //입출력을 위한 표준 라이브러리
#include <stdlib.h> //프로세스 종료를 위한 표준 라이브러리
#include <unistd.h> //lseek() -> 파일의 오프셋 지정하는 심볼릭 상수 사용시(SEEK_END, ..)
#include <fcntl.h> //파일의 용도를 결정하는 oflag 인자 정의
#include <sys/types.h> //파일 모드의 데이터 타입(_t) 정의

int main(void)
{
    char *fname = "ssu_test.txt"; //파일 이름
    off_t fsize; //파일 크기
    int fd; //파일 오픈 성공 시 반환될 파일 디스크립터

    if((fd = open(fname, O_RDONLY)) < 0) { //해당 파일을 읽기 전용으로 오픈
        fprintf(stderr, "open error for %s\n", fname); //실패 시 에러 메시지 출력
        exit(1); //에러로 인한 프로세스 종료
    }
    
    //파일의 끝 위치, 즉 파일의 크기를 반환 
    if((fsize = lseek(fd, (off_t)0, SEEK_END)) < 0) { //오프셋 위치 변경 실패할 경우
        fprintf(stderr, "lseek error\n"); //에러 메시지 출력
        exit(1); //에러로 인한 프로세스 종료
    }

    printf("The size of <%s> is %ld bytes.\n", fname, fsize); //파일명과 크기 출력

    exit(0); //정상 종료
}
