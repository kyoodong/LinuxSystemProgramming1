#include <stdio.h> //입출력을 위한 표준 라이브러리
#include <stdlib.h> //프로세스 종료를 위한 표준 라이브러리
#include <unistd.h> //lseek() -> 파일의 오프셋 지정하는 심볼릭 상수 사용시
#include <fcntl.h> //creat() -> 파일 모드의 데이터 타입(_t) 정의
#include <sys/stat.h> //파일의 접근 모드를 결정하는 mode 인자 정의

/* 파일 생성 시 접근 권한을 나타나는 mode 인자
   S_IRUSR : 파일 소유자 읽기 권한   S_IWUSR : 파일 소유자 쓰기 권한
   S_IRGRP : 그룹 사용자 읽기 권한   S_IROTH : 다른 사용자 읽기 권한 */
#define CREAT_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

char buf1[] = "1234567890";
char buf2[] = "ABCDEFGHIJ";

int main(void)
{
    char *fname = "ssu_hole.txt"; //파일 이름
    int fd; //파일 생성 성공 시 반환될 파일 디스크립터

    //매크로로 지정된 권한으로 ssu_hole.txt 파일 생성
    if((fd = creat(fname, CREAT_MODE)) < 0) { //파일 생성 실패할 경우
        fprintf(stderr, "creat error for %s\n", fname); //에러 메시지 출력
        exit(1); //에러로 인한 프로세스 종료
    }

    if(write(fd, buf1, 12) != 12) { //파일의 첫 부분에 buf1의 내용 쓰기
        fprintf(stderr, "buf1 write error\n"); //쓰기 실패할 경우 에러 메시지 출력
        exit(1);
    }

    if(lseek(fd, 15000, SEEK_SET) < 0) { //파일의 시작부터 15000만큼 건너 뛰기
        fprintf(stderr, "lseek error\n"); //오프셋 위치 변경 실패할 경우 에러 메시지 출력
        exit(1);
    }

    if(write(fd, buf2, 12) != 12) { //파일 포인터 옮겨진 곳으로부터 buf2의 내용 쓰기
        fprintf(stderr, "buf2 write error\n"); //쓰기 실패할 경우
        exit(1);
    }

    exit(0); //정상 종료
}
