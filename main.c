// 표준 입출력
#include <stdio.h>
// 표준 라이브러리
#include <stdlib.h>
// POSFIX 라이브러리
#include <unistd.h>
// 시스템 시간 라이브러리
#include <sys/time.h>
// 내가 안만듬
#include "ssu_score.h"

// 초 -> 마이크로초
#define SECOND_TO_MICRO 1000000


int main(int argc, char *argv[])
{
	ssu_score(argc, argv);
	exit(0);
}
