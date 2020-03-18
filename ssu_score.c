#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

// 점수 표
struct ssu_scoreTable score_table[QNUM];

// 학생들의 학번 표
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char cIDs[ARGNUM][FILELEN];

int eOption = false;
int tOption = false;
int pOption = false;
int cOption = false;

// ssu_score 의 메인함수
void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){
		// 도움말 출력
		if(!strcmp(argv[i], "-h")){
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);
	
	// -c 옵션이 아니면 stuDir, ansDir 을 설정
	// -c 옵션이 뭐지??
	if(argc >= 3 && strcmp(argv[1], "-c") != 0){
		strcpy(stuDir, argv[1]);
		strcpy(ansDir, argv[2]);
	}

	// 옵션 검사 후 알 수 없는 옵션 있으면 프로그램 종료
	if(!check_option(argc, argv))
		exit(1);

    // c 옵션만 있는 경우
	if(!eOption && !tOption && !pOption && cOption){
		do_cOption(cIDs);
		return;
	}

    // getcwd : 작업 디렉토리 이름을 저장해주는 함수
    // 현재 작업 디렉토리 이름을 saved_path 에 저장
	getcwd(saved_path, BUFLEN);

    // chdir : 현재 작업 디렉토리를 바꿔주는 함수
    // 현재 작업 디렉토리를 학생 디렉토리로 옮겨보고, 에러 발생 시 종료
	if(chdir(stuDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
    
	getcwd(stuDir, BUFLEN);

    // 작업 디렉토리를 다시 원래 디렉토리로 돌려놓음
	chdir(saved_path);
    
    // 현재 작업 디렉토리를 답안 디렉토리로 옮겨보고, 에러 발생 시 종료
	if(chdir(ansDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);

    // 작업 디렉토리를 다시 원래 디렉토리로 돌려놓음
	chdir(saved_path);

    // 점수 표 파일 생성
	set_scoreTable(ansDir);
    
    // 모든 학생의 학번을 알 수 있는 id_table 생성
	set_idTable(stuDir);

	printf("grading student's test papers..\n");
	score_students();

	if(cOption)
		do_cOption(cIDs);

	return;
}

/**
 옵션을 체크하는 함수
 @Param argc = 옵션 아규먼트 갯수
 @Param argv = 옵션 아규먼트 배열
 */
int check_option(int argc, char *argv[])
{
	int i, j;
	int c;

	// -e filename
	// -t, -h, -p, -c 라는 옵션을 받을 수 있음
    // @TODO: != -1 보다는 != EOF 가 더 좋아보임
	while((c = getopt(argc, argv, "e:thpc")) != -1)
	{
		switch(c){
			// -e : 에러 파일 출력
			case 'e':
				eOption = true;
				strcpy(errorDir, optarg);

                // 에러 디렉토리가 존재하는지 확인
                // 없으면 만들고 있으면 지우고 다시 만듬
				if(access(errorDir, F_OK) < 0)
					mkdir(errorDir, 0755);
				else{
					rmdirs(errorDir);
					mkdir(errorDir, 0755);
				}
				break;
			// 문제 번호 지정 시 해당 문제를 채점할 때는 -lpthread 모드 사용
			case 't':
				tOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){
                    // ARGNUM개 문제 지정 가능
					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(threadFiles[j], argv[i]);
					i++; 
					j++;
				}
				break;
                
                // p 옵션이 있나?
			case 'p':
				pOption = true;
				break;
                
                // c 옵션도 없을듯?
			case 'c':
				cOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(cIDs[j], argv[i]);
					i++; 
					j++;
				}
				break;
			case '?':
                // 알 수 없는 옵션
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}


void do_cOption(char (*ids)[FILELEN])
{
	FILE *fp;
	char tmp[BUFLEN];
	int i = 0;
	char *p, *saved;

	if((fp = fopen("score.csv", "r")) == NULL){
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}

	fscanf(fp, "%s\n", tmp);

	while(fscanf(fp, "%s\n", tmp) != EOF)
	{
		p = strtok(tmp, ",");

		if(!is_exist(ids, tmp))
			continue;

		printf("%s's score : ", tmp);

		while((p = strtok(NULL, ",")) != NULL)
			saved = p;

		printf("%s\n", saved);
	}
	fclose(fp);
}

int is_exist(char (*src)[FILELEN], char *target)
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)
			return false;
		else if(!strcmp(src[i], ""))
			return false;
		else if(!strcmp(src[i++], target))
			return true;
	}
	return false;
}

/**
 점수 표를 작성하는 함수
 @param ansDir 답안 디렉토리 경로
 */
void set_scoreTable(char *ansDir)
{
	char filename[FILELEN];

	sprintf(filename, "%s/%s", ansDir, "score_table.csv");

    // 점수 표 파일이 존재하는 지 확인하여 존재하면 읽음
    // @TODO: 기왕 검사하는거 읽기 권한이 있는지 확인하는게 좋은거 같음
	if(access(filename, F_OK) == 0)
		read_scoreTable(filename);
    
    // 점수 표 파일이 존재하지 않으면 새로 만듦
	else{
		make_scoreTable(ansDir);
		write_scoreTable(filename);
	}
}

/**
 점수 표 파일을 읽는 함수
 @param path 점수 표 파일 경로
 */
void read_scoreTable(char *path)
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){
		fprintf(stderr, "file open error for %s\n", path);
		return;
	}

    // 점수 표 파일이 1-1.txt, 0.1\n 의 꼴로 이루어져 있음
    // 따라서 %[^,] 를 통해 , 이전 까지인 1-1.txt를 qname 에 읽어내고
    // , 는 버린 뒤 나머지 0.1을 score로 읽어낸 뒤 저장 후 개행문자까지 읽어냄(버퍼에서 비움)
	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){
        // 읽어낸 qname, score 정보는 score_table 배열에 하나씩 저장
		strcpy(score_table[idx].qname, qname);
		score_table[idx++].score = atof(score);
	}

	fclose(fp);
}

/**
 점수 표 파일을 만들어주는 함수
 @param ansDir 답안 디렉토리 경로 (점수 표 파일의 상위 디렉토리)
 */
void make_scoreTable(char *ansDir)
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp, *c_dirp;
	DIR *dp, *c_dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

    // 안내 메시지 출력
	num = get_create_type();

    // 1번 선택
	if(num == 1)
	{
        // 빈칸 채우기 문제 점수 입력
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);
        
        // 프로그램 문제 점수 입력
		printf("Input value of program question : ");
		scanf("%lf", &pscore);
	}

    // opendir : 이름이 ${ansDir} 인 디렉토리를 열고 스트림 포인터를 리턴하는 함수
    // 즉 해당 스트림 포인터가 NULL 이라는건 디렉토리를 열지 못했음을 의미
	if((dp = opendir(ansDir)) == NULL){
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

    // readdir : 디렉토리 엔트리를 하나 읽은 뒤 해당 엔트리를 표현하는 포인터를 리턴함
    // 즉 내부 파일을 하나씩 읽는다고 생각하면 됨
    // 더 이상 읽을 엔트리(파일)이 없는 경우 NULL 을 리턴
    // 즉 ansDir 내부의 모든 엔트리를 읽음
	while((dirp = readdir(dp)) != NULL)
	{
        // 자기 자신을 의미하는 .과 부모 디렉토리를 의미하는 ..은 예외
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", ansDir, dirp->d_name);

        // ? 여기가 이해가 안되네 ans 안에 폴더들이 있나? 파일 밖에 없지 않나
		if((c_dp = opendir(tmp)) == NULL){
			fprintf(stderr, "open dir error for %s\n", tmp);
			return;
		}

		while((c_dirp = readdir(c_dp)) != NULL)
		{
			if(!strcmp(c_dirp->d_name, ".") || !strcmp(c_dirp->d_name, ".."))
				continue;

            // type = C 파일인지 txt 파일인지 체크
            // -1 인 경우 둘 다 아닌 다른 파일
			if((type = get_file_type(c_dirp->d_name)) < 0)
				continue;

            // .c 파일 / .txt 파일의 파일 명은 score_table 에 쌓아둠ㅁ
			strcpy(score_table[idx++].qname, c_dirp->d_name);
		}

		closedir(c_dp);
	}

	closedir(dp);
    
    // 정렬
	sort_scoreTable(idx);

    // 모든 문제에 대해 각 문제별 점수 입력
    // 1번 선택한 경우 그냥 for 문 막 돌면서 알아서 지정됨
    // 2번 선택한 경우 문제 이름 출력되면 배점을 입력해줘야함
	for(i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname);

        // 빈칸 채우기 문제 + 코드 작성 문제
		if(num == 1)
		{
            // 빈칸 채우기 문제 파일
			if(type == TEXTFILE)
				score = bscore;
            
            // 프로그래밍 문제
			else if(type == CFILE)
				score = pscore;
		}
        
        // 이 2가 대체 뭔지 모르겠소
		else if(num == 2)
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
		}

		score_table[i].score = score;
	}
}

/**
 점수 표를 작성해주는 함수
 @param filename 점수표 파일명 (score_table.csv)
 */
void write_scoreTable(char *filename)
{
	int fd;
	char tmp[BUFLEN];
	int i;
    // 스코어 테이블 내 문제 총 갯수X, 배열 전체 길이
	int num = sizeof(score_table) / sizeof(score_table[0]);

    // creat : 쓰기 전용으로 열린 파일 디스크립터를 리턴하는 함수
    // 첫 번째 매개변수는 열 파일의 이름, 두 번 째 매개변수는 접근권한
	if((fd = creat(filename, 0666)) < 0) {
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
        // 점수가 0이라는건 더이상 저장된 문제가 없다는 뜻임
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);
        
        // write : 파일 디스크립터에 tmp 문자열을 strlen(tmp) bytes 만큼 쓴다.
		write(fd, tmp, strlen(tmp));
	}

	close(fd);
}

/**
 아이디 표를 설정해주는 함수
 @param stuDir 학생 제출 답안 디렉토리 경로
 학생 제출 답안 디렉토리에는 각 학생의 학번을 이름으로 한 디렉토리가 있음
 이 디렉토리를 모두 읽어서 모든 학생의 학번을 파악함
 */
void set_idTable(char *stuDir)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

    // 학생 제출 답안 디렉토리 열기
	if((dp = opendir(stuDir)) == NULL){
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

    // 모든 파일을 읽어들임
	while((dirp = readdir(dp)) != NULL){
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name);
        
        // stat : 파일의 크기, 권한, 생성 일시 등 ls -al 로 얻을 수 있는 정보들을 대부분 얻을 수 있음
		stat(tmp, &statbuf);

        // 현재 파일이 디렉토리이면 id_table 에 그 이름을 쌓아둠
		if(S_ISDIR(statbuf.st_mode))
			strcpy(id_table[num++], dirp->d_name);
		else
			continue;
	}

	sort_idTable(num);
}

/**
 아이디 테이블을 정렬해주는 함수
 @param size 학생의 수
 */
void sort_idTable(int size)
{
	int i, j;
	char tmp[10];

    // 버블 정렬
	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
            // 오름차순 정렬
			if(strcmp(id_table[j], id_table[j+1]) > 0){
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

/**
 점수 표를 정렬해주는 함수
 @param size 점수 표의 길이
 */
void sort_scoreTable(int size)
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

    // 버블 정렬
	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);

            // 오름차순 정렬
			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){
				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

/**
 문제 번호를 뽑아내주는 함수
 @param qname 문제 파일 이름
 @param num1 문제 대번호
 @param num2 문제 소번호
 즉 1-3.txt 인 경우 num1 에는 1, num2 에는 3이 저장되어 리턴된다.
 2.c 의 경우 num1 에는 2, num2 에는 0이 저장되어 리턴된다.
 */
void get_qname_number(char *qname, int *num1, int *num2)
{
	char *p;
	char dup[FILELEN];

    // strncpy : strcpy 와 유사하지만 주어진 길이 만큼의 문자열만 복사할 수 있음
    // 즉 qname -> dup 에 널문자까지만 딱 복사된다.
	strncpy(dup, qname, strlen(qname));
    
    // strtok : String tokenizer 함수로, 두 번째 인자 값을 토큰으로 문자열을 잘라준다.
    // -. 이라고해서 딱 -.만 구분해주는게 아니라 "-", ".", "-." 모두 잘라준다.
	*num1 = atoi(strtok(dup, "-."));
	
    // NULL 을 첫 번재 매개변수로 넣게되면 직전에 사용했던 문자열(dup)을 이어서 자르게 된다.
    // NULL이 아닌 dup 을 적어주게되면 계속해서 첫 번째 구간의 문자열만 받게 될 것이다.
	p = strtok(NULL, "-.");
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p);
}

/**
 점수 표 작성 방식에 대해 사용자 안내문을 출력하고 입력받는 함수
 @return 점수표 작성 방식
    1 : 빈칸 문제와 프로그램 문제의 점수만 입력
    2 : 모든 문제의 점수
 */
int get_create_type()
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist in TREUDIR!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

        // 1 또는 2만 입력받도록 함 (아닐 경우 무한반복)
		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

/**
 학생들의 점수를 매기는 함수
 */
void score_students()
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]);

    // score.csv 를 쓰기모드로 열기
    // 주의 : 쓰기모드라고 함은 안에 있는 내용을 이어 쓰는 것이 아니라 연 순간 백지상태로 돌아감을 말한다.
	if((fd = creat("score.csv", 0666)) < 0){
		fprintf(stderr, "creat error for score.csv");
		return;
	}
    
    // 문제 리스트(column name) 명시
    // 일종의 표 형태를 생각하면 좋음. 표 맨 위에 어떤 컬럼이 있는지 명시하듯 그 작업을 한 것
	write_first_row(fd);

    // 모든 학번을 두 번째 행에 쭉 명시
    // 예) 20162489, 20162490, 20162491, ...
    // 그러면서
	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], ""))
			break;

		sprintf(tmp, "%s,", id_table[num]);
		write(fd, tmp, strlen(tmp)); 

		score += score_student(fd, id_table[num]);
	}

	if(pOption)
		printf("Total average : %.2f\n", score / num);

	close(fd);
}

/**
 학생 점수 매기는 함수
 @param fd score.csv file descriptor
 @param id 학번
 @return 학생 점수
 */
double score_student(int fd, char *id)
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

    // 모든 문제 loop
	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)
			break;

        // 학생답안/학번/문제
		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname);

        // 파일이 존재하지 않음
		if(access(tmp, F_OK) < 0)
			result = false;
		else    // 파일 존재
		{
            // .c .txt 파일 둘 다 아닌 경우 넘어감
			if((type = get_file_type(score_table[i].qname)) < 0)
				continue;
			
            // .txt 파일인 경우
			if(type == TEXTFILE)
				result = score_blank(id, score_table[i].qname);
            
            // .c 파일인 경우
			else if(type == CFILE)
				result = score_program(id, score_table[i].qname);
		}

		if(result == false)
			write(fd, "0,", 2);
		else{
			if(result == true){
				score += score_table[i].score;
				sprintf(tmp, "%.2f,", score_table[i].score);
			}
			else if(result < 0){
				score = score + score_table[i].score + result;
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp));
		}
	}

	if(pOption)
		printf("%s is finished.. score : %.2f\n", id, score); 
	else
		printf("%s is finished..\n", id);

	sprintf(tmp, "%.2f\n", score);
	write(fd, tmp, strlen(tmp));

	return score;
}

/**
 첫 번째 행을 작성하는 함수
 @param fd score.csv 의 file descriptor
 첫 번째 행에 column name 을 명시해주는 작업이 이루어짐
 예) ,1-1, 1-2, 1-3, 2-1, 2-2, sum
 */
void write_first_row(int fd)
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

    // ?? 왜 ,로 시작하는거지
	write(fd, ",", 1);

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0)
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname);
		write(fd, tmp, strlen(tmp));
	}
	write(fd, "sum\n", 4);
}

/**
 학생 답안 파일에서 답을 읽어내는 함수
 @param fd 학생 답안 파일 file descriptor
 @param result 결과를 담을 문자열 (학생 답안이 result 에 저장됨)
 @return 학생 답안
 */
char *get_answer(int fd, char *result)
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);
    
    // 파일의 끝 또는 :를 만날때까지 한 글자씩 읽음
	while(read(fd, &c, 1) > 0)
	{
        // 학생 답안 파일에 : 은 왜 있는걸까~?
		if(c == ':')
			break;
		
		result[idx++] = c;
	}
	if(result[strlen(result) - 1] == '\n')
		result[strlen(result) - 1] = '\0';

	return result;
}

/**
 빈칸 채우기 문제 채점하는 함수
 @param id 학번
 @param filename 채점할 답안 파일 경로
 @return
 */
int score_blank(char *id, char *filename)
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

    // qname 에 .txt 를 제외한 파일명 복사
    // 예) filename = 1-1.txt
    // qname = 1-1
	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename);
	fd_std = open(tmp, O_RDONLY);
    
    // strcpy 왜한거지??
	strcpy(s_answer, get_answer(fd_std, s_answer));

	if(!strcmp(s_answer, "")){
		close(fd_std);
		return false;
	}

    // 괄호 validation check
	if(!check_brackets(s_answer)){
		close(fd_std);
		return false;
	}

    // 양쪽 공백을 지움
    // 예) " abc   " -> "abc"
	strcpy(s_answer, ltrim(rtrim(s_answer)));

    // 세미콜론 검사 후 제거
	if(s_answer[strlen(s_answer) - 1] == ';'){
		has_semicolon = true;
		s_answer[strlen(s_answer) - 1] = '\0';
	}

    // 토큰 쪼개기
	if(!make_tokens(s_answer, tokens)){
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0);

	sprintf(tmp, "%s/%s/%s", ansDir, qname, filename);
	fd_ans = open(tmp, O_RDONLY);

	while(1)
	{
		ans_root = NULL;
		result = true;

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		strcpy(a_answer, get_answer(fd_ans, a_answer));

		if(!strcmp(a_answer, ""))
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer)));

		if(has_semicolon == false){
			if(a_answer[strlen(a_answer) -1] == ';')
				continue;
		}

		else if(has_semicolon == true)
		{
			if(a_answer[strlen(a_answer) - 1] != ';')
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0';
		}

		if(!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0);

		compare_tree(std_root, ans_root, &result);

		if(result == true){
			close(fd_std);
			close(fd_ans);

			if(std_root != NULL)
				free_node(std_root);
			if(ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}
	
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL)
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char *id, char *filename)
{
	double compile;
	int result;

	compile = compile_program(id, filename);

	if(compile == ERROR || compile == false)
		return false;
	
	result = execute_program(id, filename);

	if(!result)
		return false;

	if(compile < 0)
		return compile;

	return true;
}

int is_thread(char *qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname))
			return true;
	}
	return false;
}

double compile_program(char *id, char *filename)
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
	
	isthread = is_thread(qname);

	sprintf(tmp_f, "%s/%s/%s", ansDir, qname, filename);
	sprintf(tmp_e, "%s/%s/%s.exe", ansDir, qname, qname);

	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_e, "%s/%s/%s_error.txt", ansDir, qname, qname);
	fd = creat(tmp_e, 0666);

	redirection(command, fd, STDERR);
	size = lseek(fd, 0, SEEK_END);
	close(fd);
	unlink(tmp_e);

	if(size > 0)
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename);
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname);

	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname);
	fd = creat(tmp_f, 0666);

	redirection(command, fd, STDERR);
	size = lseek(fd, 0, SEEK_END);
	close(fd);

	if(size > 0){
		if(eOption)
		{
			sprintf(tmp_e, "%s/%s", errorDir, id);
			if(access(tmp_e, F_OK) < 0)
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname);
			rename(tmp_f, tmp_e);

			result = check_error_warning(tmp_e);
		}
		else{ 
			result = check_error_warning(tmp_f);
			unlink(tmp_f);
		}

		return result;
	}

	unlink(tmp_f);
	return true;
}

double check_error_warning(char *filename)
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){
		if(!strcmp(tmp, "error:"))
			return ERROR;
		else if(!strcmp(tmp, "warning:"))
			warning += WARNING;
	}

	return warning;
}

int execute_program(char *id, char *filename)
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	sprintf(ans_fname, "%s/%s/%s.stdout", ansDir, qname, qname);
	fd = creat(ans_fname, 0666);

	sprintf(tmp, "%s/%s/%s.exe", ansDir, qname, qname);
	redirection(tmp, fd, STDOUT);
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname);
	fd = creat(std_fname, 0666);

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname);

	start = time(NULL);
	redirection(tmp, fd, STDOUT);
	
	sprintf(tmp, "%s.stdexe", qname);
	while((pid = inBackground(tmp)) > 0){
		end = time(NULL);

		if(difftime(end, start) > OVER){
			kill(pid, SIGKILL);
			close(fd);
			return false;
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname);
}

pid_t inBackground(char *name)
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp));
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);

	sprintf(command, "ps | grep %s", name);
	redirection(command, fd, STDOUT);

	lseek(fd, 0, SEEK_SET);
	read(fd, tmp, sizeof(tmp));

	if(!strcmp(tmp, "")){
		unlink("background.txt");
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " "));
	close(fd);

	unlink("background.txt");
	return pid;
}

int compare_resultfile(char *file1, char *file2)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY);
	fd2 = open(file2, O_RDONLY);

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){
			if(c1 == ' ') 
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){
			if(c2 == ' ') 
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0)
			break;

		to_lower_case(&c1);
		to_lower_case(&c2);

		if(c1 != c2){
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;
}

void redirection(char *command, int new, int old)
{
	int saved;

	saved = dup(old);
	dup2(new, old);

	system(command);

	dup2(saved, old);
	close(saved);
}

/**
 파일 타입을 알려주는 함수
 @param filename 파일 이름
 @return TEXTFILE : .txt 파일
    CFILE : .c 파일
    -1 : 둘 다 아님
 */
int get_file_type(char *filename)
{
    // .을 문자열 뒤에서부터 검색
	char *extension = strrchr(filename, '.');

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

/**
 디렉토리를 지우는 함수
 @param path 디렉토리 경로
 */
void rmdirs(const char *path)
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[50];
	
    // 디렉토리 스트리 열기
	if((dp = opendir(path)) == NULL)
		return;

    // 디렉토리(dp) 하위에 있는 파일 및 디렉토리 정보를 한 건 읽음
    // while 문 안에 있으므로 모든 파일 및 디렉토리를 읽게 됨
	while((dirp = readdir(dp)) != NULL)
	{
        // 파일 이름이 . 이나 .. 인 경우 넘어감
        // 리눅스, 유닉스에서 ls -a 하면 자기 자신과 부모 디렉토리를 의미하는 ., .. 파일을 말함
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

        // 디렉토리명/파일명 의 형태로 문자열 구성
		sprintf(tmp, "%s/%s", path, dirp->d_name);

        // 파일의 상태를 알아내기 위함
        // 크기, 권한, 생성일시, 최종 변경일 등
		if(lstat(tmp, &statbuf) == -1)
			continue;

        // st_mode : 파일의 종류 및 접근 권한
        // S_ISDIR : 파일의 종류가 디렉토리인지 확인해주는 전처리 함수
        // 파일의 종류가 디렉토리라면 디렉토리를 지움
        // 아니라면 unlink
        // @TODO: remove 함수를 쓰면 디렉토리일 때와 파일일 때를 자동으로 구분해준다고 한다.
		if(S_ISDIR(statbuf.st_mode))
			rmdirs(tmp);
		else
			unlink(tmp);
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char *c)
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage()
{
	printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -h                print usage\n");
	printf(" -p                print student's score and total average\n");
	printf(" -c <IDS>          print ID's score\n");
}
