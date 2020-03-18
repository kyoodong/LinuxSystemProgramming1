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


void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

int main(int argc, char *argv[])
{
	// 시작시간
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);
	ssu_score(argc, argv);

	// 끝 시간
	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);

	exit(0);
}

// 프로그램의 총 실행 시간을 출력하는 함수
// @Param begin_t = 프로그램 시작 시간
// @Param end_t = 프로그램 종료 시간 
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)
{
	end_t->tv_sec -= begin_t->tv_sec;

	// 마이크로 초 계산 보정
	// 시작 시간이 5.8초, 끝 시간이 6.3 초 일 때 시차는 0.5초이지만 이 구문이 없으면 1.5초로 구해짐
	if(end_t->tv_usec < begin_t->tv_usec){
		end_t->tv_sec--;
		end_t->tv_usec += SECOND_TO_MICRO;
	}

	end_t->tv_usec -= begin_t->tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}
