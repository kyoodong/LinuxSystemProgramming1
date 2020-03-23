#include <stdio.h> //입출력을 위한 표준 라이브러리
#include <stdlib.h> //프로세스 종료를 위한 표준 라이브러리
#include <unistd.h> //파일 디스크립터를 위한 심볼릭 상수 사용시(0, 1, 2), close() 사용시
#include <fcntl.h> //creat() 함수 사용시
#include <sys/stat.h> //파일의 접근 모드를 결정하는 mode 인자 정의
#include <sys/types.h> //파일 모드의 데이터 타입(_t) 정의

int main(void)
{
    char *fname = "ssu_test.txt"; //파일 이름
    int fd; //파일 생성 성공 시 반환될 파일 디스크립터

    //파일의 접근 권한 모드를 0666으로 파일 생성
    //0666(rw-rw-rw-) : 유저, 그룹, 다른 사용자가 모두 읽고 쓸 수 있음
    if((fd = creat(fname, 0666)) < 0) {
        fprintf(stderr, "creat error for %s\n", fname); //생성 실패할 경우 에러 메시지 출력
        exit(1); //에러로 인한 프로세스 종료
    }
    else { //파일 생성 성공할 경우
        printf("Success!\nfilename : %s\nDescriptor : %d\n", fname, fd);
        close(fd); //파일 닫기
    }

    exit(0); //정상 종료
}
