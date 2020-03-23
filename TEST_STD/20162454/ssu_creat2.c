#include <stdio.h> //입출력을 위한 표준 라이브러리
#include <stdlib.h> //프로세스 종료를 위한 표준 라이브러리
#include <unistd.h> //close() 함수 사용시
#include <fcntl.h> //creat(), open() 함수 사용시
#include <sys/stat.h> //파일의 접근 모드를 결정하는 mode 인자 정의
#include <sys/types.h> //파일 모드의 데이터 타입(_t) 정의

int main(void)
{
    char *fname = "ssu_test.txt"; //파일 이름
    int fd; //파일 생성 성공 시 반환될 파일 디스크립터
    
    //파일의 접근 권한 모드를 0666로 하여 파일 생성
    if((fd = creat(fname, 0666)) < 0) {
        fprintf(stderr, "creat error for %s\n", fname); //생성 실패할 경우 에러 메시지 출력
        exit(1); //에러로 인한 프로세스 종료
    }
    else {
        close(fd); //파일 닫기
        fd = open(fname, O_RDWR); //읽고 쓰기 가능하도록 파일 오픈
        printf("Succeeded!\n<%s> is new readable and writable\n", fname);
    }

    exit(0); //정상 종료

}
