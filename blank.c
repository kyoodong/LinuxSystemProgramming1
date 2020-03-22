#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"};


// 연산자와 그 우선순위를 표현함 (수가 낮으면 낮을 수록 우선순위가 높은 것임)
operator_precedence operators[OPERATOR_CNT] = {
	{"(", 0}, {")", 0}
	,{"->", 1}	
	,{"*", 4}	,{"/", 3}	,{"%", 2}	
	,{"+", 6}	,{"-", 5}	
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};

/**
 두 트리가 같은 결과를 내는지 확인하는 함수
 @param root1 트리 루트 노드
 @param root2 트리 루트 노드
 @param result true : 같음
				false : 다름
 */
void compare_tree(node *root1,  node *root2, int *result)
{
	node *tmp;

	// 둘 중 하나라도 NULL이면 false
	if(root1 == NULL || root2 == NULL){
		*result = false;
		return;
	}

	// 비교 연산자는 피연산자가 왼쪽에 있냐 오른쪽에 있냐에 따라 괄호의 방향을 바꾸기만하면 같은 결과를 내기에 이에 대한 처리
	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){
		if(strcmp(root1->name, root2->name) != 0){

			if(!strncmp(root2->name, "<", 1))
				strncpy(root2->name, ">", 1);

			else if(!strncmp(root2->name, ">", 1))
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))
				strncpy(root2->name, "<=", 2);

			// 오른쪽 형제(피연산자)와 위치를 바꿈
			root2 = change_sibling(root2);
		}
	}

	// 두 root의 이름이 다르면 false
	if(strcmp(root1->name, root2->name) != 0){
		*result = false;
		return;
	}

	// 한쪽만 자식 노드가 있는 경우 false
	if((root1->child_head != NULL && root2->child_head == NULL)
		|| (root1->child_head == NULL && root2->child_head != NULL)){
		*result = false;
		return;
	}

	else if(root1->child_head != NULL){
		// 두 자식 노드의 형제 노드의 수가 다르면 false
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!="))
		{
			compare_tree(root1->child_head, root2->child_head, result);

			// a == b;와 b == a; 를 처리하기 위함
			// change_sibling 하게 되면 a == b;와 b == a;의 트리모양이 같아지게 됨
			if(*result == false)
			{
				*result = true;
				root2 = change_sibling(root2);
				compare_tree(root1->child_head, root2->child_head, result);
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{
			// 형제노드 갯수가 다르면 false
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){
				*result = false;
				return;
			}

			tmp = root2->child_head;

			while(tmp->prev != NULL)
				tmp = tmp->prev;

			// 모든 형제들과 비교하며 같은 피연산자를 찾아서 하나라도 같으면 성공
			while(tmp != NULL)
			{
				compare_tree(root1->child_head, tmp, result);
			
				if(*result == true)
					break;
				else{
					if(tmp->next != NULL)
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		// ==, !=, +, *, ||, |, &&, & 이외의 문자
		else{
			compare_tree(root1->child_head, root2->child_head, result);
		}
	}

	// 다음 형제 노드가 있는 경우
	if(root1->next != NULL){
		// 형제 노드 수 다르면 false
		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){
			*result = false;
			return;
		}

		if(*result == true)
		{
			tmp = get_operator(root1);
	
			// 이 연산자들은 피연산자 간의 순서가 바뀌어도 무관한 연산자들임
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{
				tmp = root2;
	
				while(tmp->prev != NULL)
					tmp = tmp->prev;

				// 형제 노드들 중에 피연산자가 같은게 하나라도 있으면 성공
				while(tmp != NULL)
				{
					compare_tree(root1->next, tmp, result);

					if(*result == true)
						break;
					else{
						if(tmp->next != NULL)
							*result = true;
						tmp = tmp->next;
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result);
		}
	}
}

/**
 토큰을 만들어주는 함수
 @param str 학생 답안 문자열 (양 쪽 공백과 세미콜론이 제거된 상태)
 @param tokens 만들어진 토큰을 저장하는 배열
 @return 0 잘못된 구문(str)
		 1 문제 없는 구문(str)
 */
int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])
{
	char *start, *end;
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; 
	int row = 0;
	int i;
 	int isPointer;
	int lcount, rcount;
	int p_str;
	
	clear_tokens(tokens);

	start = str;
	
	// 선언문 또는 일반 구문인지 확인
	// 0인 경우는 잘못된 입력
	if(is_typeStatement(str) == 0) 
		return false;	
	
	while(1)
	{
		// 연산자를 찾아 보고, 더 이상의 연산자가 없으면 끝냄
		if((end = strpbrk(start, op)) == NULL)
			break;

		// 연산자 읽어야 하는 상황
		if(start == end) {
			// ++ 또는 --로 시작
			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){
				// ++++, ---- 는 잘못된 입력
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4))
					return false;

				// ++ 또는 -- 를 건너 뛰고 공백이 아닌 첫 번째 글자가 알파벳 또는 숫자인 경우
				if(is_character(*ltrim(start + 2))){
					// 이 전 토큰의 마지막 문자가 character 이면 잘못됨
					// 예를 들어서 a++ b 이런거
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))
						return false;

					// 또 다른 연산자가 있는지 확인
					end = strpbrk(start + 2, op);
					if(end == NULL)
						end = &str[strlen(str)];
					
					// 또 다른 연산자가 있었다면 그 연산자 직전까지 반복
					// 없었다면 해당 구문의 끝까지 반복
					// 어찌됐든 이 반복을 도는 동안에는 더 이상의 연산자는 없음
					// 정리하자면 이 while문은 "-- a" 이런건 상관 없음
					// 근데 "--a  b" 이런 구문은 안됨
					while(start < end) {
						// 공백을 만났는데 토큰 끝이 숫자나 문자면 이는 잘못된 입력
						// 즉 피연산자가 두개 이상이면 잘못된 입력
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
							return false;
						// 공백이 아니면 token에 한 글자씩 누적
						// 결국 공백을 제외한 구문 전체를 토큰에 저장
						// 예) "-- a" 의 경우 토큰에 "--a"로 저장
						else if(*start != ' ')
							// strncat : 문자열 이어붙이기 함수
							strncat(tokens[row], start, 1);
						start++;	
					}
				}
				
				// 직전 토큰이 숫자나 문자로 끝난 경우
				// 즉 ++a 가 아닌 a++ 이런 경우를 찾아내기 위함
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					// 직전 토큰에 ++ 이나 -- 가 있으면 안됨
					// 이게 "++a++" 이런 구문을 방지하려고 그런듯
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL)	
						return false;

					// ++이나 --가 없는 경우이므로
					// 직전 토큰에 이어갈 수 있음
					// 결국 a, ++ 로 분리된 토큰을 a++ 로 이어붙임
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2);
					strcat(tokens[row - 1], tmp);
					start += 2;
					row--;
				}
				
				// 직전이나 직후에 피연산자가 나오지 않은 경우는 그냥 토큰에 ++이나 --를 넣어줌
				// (*a)++ 이런거
				else{
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2);
					strcat(tokens[row], tmp);
					start += 2;
				}
			}

			// 아래 연산자들은 토큰에 바로 추가
			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){

				strncpy(tokens[row], start, 2);
				start += 2;
			}
			// 포인터 멤버 변수 연산자
			else if(!strncmp(start, "->", 2))
			{
				end = strpbrk(start + 2, op);

				if(end == NULL)
					end = &str[strlen(str)];

				// -> 연산자와 -> 이후에 등장하는 최초의 피연산자를 토큰에 추가
				// @TODO: 여기는 또 피연산자 두 개 이상인거 안잡네
				while(start < end){
					if(*start != ' ')
						strncat(tokens[row - 1], start, 1);
					start++;
				}
				row--;
			}
			// 갑자기 if 문에 start가 아닌 end가 등장하는데 애초에 여기는 start == end if문을 통과해야 들어올 수 있는 곳이라 상관없음
			// 연산자가 &인 경우
			// &&는 위에서 처리됐으므로 bitwise & 혹은 주소값연산자만 고려함
			else if(*end == '&')
			{
				// 0 번째 토큰을 입력할 차례거나 직전 토큰에 연산자가 있는 경우
				// 주소값을 의미하는 &
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){
					// & 이후의 피연산자를 찾는 로직
					end = strpbrk(start + 1, op);
					if(end == NULL)
						end = &str[strlen(str)];
					
					// 토큰에 & 추가하고 한 글자 넘어감
					strncat(tokens[row], start, 1);
					start++;

					// 다음 연산자 직전까지이든 문자열의 맨 끝이든 이 while 문은 피연산자만 다루게 됨
					// & 연산자 이후의 피연산자를 토큰에 이어 붙임
					while(start < end){
						// && 같이 &가 두 번 연속 등장하는거 막는건가?
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&')
							return false;
						else if(*start != ' ')
							strncat(tokens[row], start, 1);
						start++;
					}
				}
				
				// bitwise &
				else{
					// 토큰에 '&'연산자 이어 붙이고 start 증가
					strncpy(tokens[row], start, 1);
					start += 1;
				}
				
			}
			// 연산자가 *인 경우
		  	else if(*end == '*')
			{
				isPointer=0;

				if(row > 0)
				{
					// 직전 토큰에 데이터 형을 의미하는 토큰이 있다면 이는 포인터 변수 선언임
					// 예) int*
					// @TODO: row-- 왜 안하지?
					for(i = 0; i < DATATYPE_SIZE; i++) {
						if(strstr(tokens[row - 1], datatype[i]) != NULL){
							strcat(tokens[row - 1], "*");
							start += 1;	
							isPointer = 1;
							break;
						}
					}
					
					// 포인터면 바로 이후 과정 생략
					if(isPointer == 1)
						continue;
					
					// * 다음에 어떤 문자가 존재한다면 end를 옮김 (공백도 가능)
					if(*(start + 1) != 0)
						end = start + 1;
					
					// 전전 토큰이 '*'이고, 전 토큰이 전부 *로 이루어진 문자열이면 start의 *을 직전 토큰에 이어붙임
					// 예를 들면 b가 3차원 배열이고 원본 문자열이 a = ***b * c; 이런 구문일 때 **b를 묶어내줌
					// @TODO: 하나 불안한건 ***b에서 맨 앞에 *은 안묶이는데 이게 트리에서 곱하기 연산자로 쓰일거 같음 따로 처리해줘야하지않나싶다
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){
						strncat(tokens[row - 1], start, end - start);
						row--;
					}
					
					// 직전 토큰이 문자 또는 숫자로 끝났다면 토큰에 *을 넣음
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){ 
						strncat(tokens[row], start, end - start);   
					}

					// 직전 토큰에 연산자가 있어도 토큰에 *을 넣음
					else if(strpbrk(tokens[row - 1], op) != NULL){		
						strncat(tokens[row] , start, end - start); 
							
					}
					// @TODO: 뭐야 결국 넣잖아?
					else
						strncat(tokens[row], start, end - start);

					start += (end - start);
				}

				// 포인터
				// 예) *p = 10;
			 	else if(row == 0)
				{
					// 더이상 연산자가 없으면 토큰에 start 추가
					if((end = strpbrk(start + 1, op)) == NULL){
						strncat(tokens[row], start, 1);
						start += 1;
					}
					// 연산자가 더 있는 경우
					else {
						// 해당 연산자 전까지의 피연산자를 토큰에 누적
						while(start < end){
							// *p  = 10; 같이 피연산자와 연산자 사이에 공백이 두 개 이상이면 걸리긴함?
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
								return false;
							else if(*start != ' ')
								strncat(tokens[row], start, 1);
							start++;	
						}
						
						// 누적한 피연산자가 모두 *이면 취소
						if(all_star(tokens[row]))
							row--;
					}
				}
			}
			else if(*end == '(') 
			{
				lcount = 0;
				rcount = 0;
				
				// 직전 토큰이 & 도는 * 인 경우
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){
					// 연속된 '('의 갯수를 셈
					while(*(end + lcount + 1) == '(')
						lcount++;
					start += lcount;

					// 가장 가까운 ')'를 찾아봄
					end = strpbrk(start + 1, ")");

					// (가 최소 하나 있으니 )도 최소 하나 있지 않으면 잘못된 수식임
					if(end == NULL)
						return false;
					else{
						// 연속된 ')'의 갯수를 셈
						while(*(end + rcount +1) == ')')
							rcount++;
						end += rcount;

						// 여는 괄호와 닫는 괄호의 갯수가 다르면 잘못된 수식임
						if(lcount != rcount)
							return false;

						// 전전 토큰이 문자나 숫자가 아닌 경우(?) 혹은 1번째 토큰인 경우 딱 필요한 괄호 하나와 피연산자만 추려서 전 토큰에 붙인다
						// 예1) *(((((((((((((a)))))))))))))) -> *a
						// 예2) int b = *((((((a)))))); -> int b = *a;
						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){ 
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1);
							row--;
							start = end + 1;
						}
						// 그냥 토큰에 ( 추가
						// 예) int a = b * (c + 10);
						else{
							strncat(tokens[row], start, 1);
							start += 1;
						}
					}
				}
				
				// 그냥 토큰에 ( 추가
				// 예) int a = b + (c + 10);
				else {
					strncat(tokens[row], start, 1);
					start += 1;
				}
			}
			// "" 큰따옴표 내부의 문자를 세트로 묶어서 토큰에 넣는다.
			else if(*end == '\"') 
			{
				end = strpbrk(start + 1, "\"");
				
				if(end == NULL)
					return false;

				else{
					strncat(tokens[row], start, end - start + 1);
					start = end + 1;
				}
			}

			// 기타 알파벳, 숫자, -, +, /, %, | 등
			else{
				if(row > 0 && !strcmp(tokens[row - 1], "++"))
					return false;
				
				if(row > 0 && !strcmp(tokens[row - 1], "--"))
					return false;
	
				strncat(tokens[row], start, 1);
				start += 1;

				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){
					if(row == 0)
						row--;

					// 직전 토큰이 문자나 숫자로 끝나지 않고 ++, -- 가 없다면 row-- (?)
					// @TODO: 이게 이러면 근데 다음 연산자는 row에 저장이 된 상태에서 row-- 되어버리면 토큰이 꼬이는 문제가 있음
					// 꼬인다는게 예를 들면 b++ * c; 이런 구문의 경우 토큰이 b++, *c 로 저장됨. 마치 c가 포인터인것처럼
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL)
							row--;
					}
				}
			}
		}
		
		// start != end
		else {
			// a = ***b * c; 이런 경우에 *, **b로 토큰을 쪼개줌 (포인터는 묶어준다는 말임)
			// @TODO: 근데 위 예시에서도 보이듯이 ***b까지가 포인터 연산인데 맨 앞에 *은 안묶어줌 이런 경우 나중에 트리에서 맨 앞의 *은 곱하기로 인식될 여지가 있음
			if(row > 1 && all_star(tokens[row - 1]) && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))
				row--;

			if(row == 1 && all_star(tokens[row - 1]))
				row--;

			// 피연산자 토큰에 추가
			// student.id 이런거 묶어서 처리
			for(i = 0; i < end - start; i++){
				if(i > 0 && *(start + i) == '.'){
					strncat(tokens[row], start + i, 1);

					while( *(start + i +1) == ' ' && i< end - start )
						i++; 
				}
				else if(start[i] == ' '){
					while(start[i] == ' ')
						i++;
					break;
				}
				else
					strncat(tokens[row], start + i, 1);
			}

			if(start[0] == ' ') {
				start += i;
				continue;
			}
			start += i;
		}
		
		// 좌우 공백 제거
		if (row >= 0)
			strcpy(tokens[row], ltrim(rtrim(tokens[row])));

		// 현재 row에 토큰이 문자나 숫자로 끝나면서 (직전 토큰이 선언문이거나 문자가 있거나 .으로 끝났)을때
		if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1])
		   && (is_typeStatement(tokens[row - 1]) == 2
			   || is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
			   || tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){
			
			// 전전 토큰이 '(' 일 때
			if(row > 1 && strcmp(tokens[row - 2],"(") == 0)
			{
				// 직전 토큰이 struct나 unsigned이여야만 함
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0)
					return false;
			}
			// row가 1이면서 1번째 토큰에 문자가 있을때
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				// 0번째 토큰이 extern, unsigned, 선언문 셋 중 하나여야만 함
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)
					return false;
			}
			// 직전 토큰이 선언문일때
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				// 전전 토큰은 unsigned 또는 extern 이여야만 함
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;
			}
		}

		// 첫 번째 토큰이면서 그 토큰이 gcc라면 토큰 다 비우고, 구문 전체를 0번째 토큰에 넣음
		if((row == 0 && !strcmp(tokens[row], "gcc")) ){
			clear_tokens(tokens);
			strcpy(tokens[0], str);	
			return 1;
		}

		row++;
		// while(1) 끝
	}

	// 직전 토큰이 모두 *이고, 전전 토큰이 문자나 숫자로 끝나지 않은 경우
	// /** 같은 전체 주석 말하는건가?
	if(row > 1 && all_star(tokens[row - 1]) && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))
		row--;
	
	// 직전 토큰이 모두 *인 경우
	if(row == 1 && all_star(tokens[row - 1]))
		row--;	

	// 맨마지막 피연산자처리
	for(i = 0; i < strlen(start); i++)   
	{
		if(start[i] == ' ')  
		{
			while(start[i] == ' ')
				i++;
			if(start[0]==' ') {
				start += i;
				i = 0;
			}
			else
				row++;
			
			i--;
		} 
		else
		{
			strncat(tokens[row], start + i, 1);
			
			// '.' 발견 시 이후 등장하는 모든 공백 제거 (?)
			// @TODO: i < strlen(start) 필요한 구문인가?
			if( start[i] == '.' && i<strlen(start)){
				while(start[i + 1] == ' ' && i < strlen(start))
					i++;

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));

		// 토큰이 lpthread 이고, 직전 토큰이 - 인 경우 즉, -lpthread
		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){
			// 직전 토큰에 -lpthread를 이어 붙이고, 현재 토큰을 비움
			strcat(tokens[row - 1], tokens[row]);
			memset(tokens[row], 0, sizeof(tokens[row]));
			row--;
		}
		// 현재 토큰에 값이 있고 (직전 토큰이 선언문이거나 문자나 숫자로 끝나거나 .으로 끝난경우)
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){
			
			// 전전 토큰이 '(' 인 경우
			if(row > 1 && strcmp(tokens[row-2],"(") == 0)
			{
				// 직전 토큰은 struct 또는 unsigned 이여야함
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0)
					return false;
			}
			
			// 1번째 토큰이 문자나 숫자로 끝난다는 것은
			// 0번째 토큰이 extern, unsigned 혹은 연산자여야 한다는 것이다.
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false;
			}
			
			// 타입 선언문이 0번째 토큰이 아닌곳에서 발견된 경우
			// 피연산자가 두 개 연속 나올 수는 없으므로
			// 두 개 연속 가능한 경우는 타입 선언문 이 전에 unsigned 혹은 extern이 나오는 경우 뿐이므로 이 둘이 아닌 경우 false
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;
			}
		}
	}


	if(row > 0)
	{
		// 첫 토큰이 #include, include, struct 셋 중 하나라면 토큰을 모두 정리하고 현재 구문을 token의 0번째에 넣는다.
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){ 
			clear_tokens(tokens); 
			strcpy(tokens[0], remove_extraspace(str)); 
		}
	}

	// 0번째 토큰이 선언문이거나 extern을 포함하고 있다면 모든 토큰을 결합
	// 예) "int a = 10;" 이런 구문이였다면
	// 토큰은 tokens[0] = "int"
	// tokens[1] = "a"
	// tokens[2] = "="
	// 이었을 텐데 이를 tokens[0] = "int a =" 으로 바꿈
	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){
		for(i = 1; i < TOKEN_CNT; i++){
			if(strcmp(tokens[i],"") == 0)  
				break;			   

			// 0번째 토큰에 공백을 추가해줌
			if(i != TOKEN_CNT -1 )
				strcat(tokens[0], " ");
			
			// i번째 토큰을 0번째 토큰에 추가
			strcat(tokens[0], tokens[i]);
			
			// i 번째 토큰을 초기화
			memset(tokens[i], 0, sizeof(tokens[i]));
		}
	}
	
	// 타입 지정자를 찾음
	while((p_str = find_typeSpecifier(tokens)) != -1){
		// 무의미한 괄호들을 없애줌
		if(!reset_tokens(p_str, tokens))
			return false;
	}

	// struct 찾아 다님
	while((p_str = find_typeSpecifier2(tokens)) != -1){  
		if(!reset_tokens(p_str, tokens))
			return false;
	}
	
	return true;
}

/**
 트리를 만들어주는 함수
 @param root 만들어진 트리의 루트가 저장될 공간
 @param tokens 토큰 테이블
 @param idx 토큰 인덱스
 @param parentheses 괄호 depth
 @return 만들어진 트리의 루트 노드
 */
node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses)
{
	node *cur = root;
	node *new;
	node *operator;
	int fstart;
	int i;

	while(1)
	{
		// 더 이상 처리할 토큰이 없으면 반복문 종료
		if(strcmp(tokens[*idx], "") == 0)
			break;
	
		// 괄호 닫기를 만나면 바로 리턴
		// 왜냐면 괄호 열기를 만나는 순간에 make_tree가 재귀적으로 호출되어 하나의 서브트리가 만들어지는 과정이였기에
		// 여기서 리턴되는 루트 노드는 서브 트리의 루트노드임
		if(!strcmp(tokens[*idx], ")"))
			return get_root(cur);

		else if(!strcmp(tokens[*idx], ","))
			return get_root(cur);

		else if(!strcmp(tokens[*idx], "("))
		{
			// 직전 토큰이 연산자 또는 ','가 아닐 때 새로운 트리를 만들어서 cur 에 이어 붙여줌
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){
				fstart = true;

				// @TODO: 두 번 이상 도는 경우가 있나?
				while(1)
				{
					*idx += 1;

					if(!strcmp(tokens[*idx], ")"))
						break;
					
					new = make_tree(NULL, tokens, idx, parentheses + 1);
					
					if(new != NULL){
						// 최초의 자식이면 child_head 에 바로 넣어주고
						if(fstart == true){
							cur->child_head = new;
							new->parent = cur;
	
							fstart = false;
						}
						// 최초는 아니라면 next에 넣어줌
						else{
							cur->next = new;
							new->prev = cur;
						}

						cur = new;
					}

					if(!strcmp(tokens[*idx], ")"))
						break;
				}
			}
			else{
				*idx += 1;
	
				// 트리를 새로 만듬
				new = make_tree(NULL, tokens, idx, parentheses + 1);

				// 현재 노드가 NULL이면 새롭게 만들어진 트리를 루트로 지정
				if(cur == NULL)
					cur = new;

				// 현재 노드와 새로운 노드가 이름이 같은 상황
				else if(!strcmp(new->name, cur->name)){
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||") 
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))
					{
						// new 의 자식 노드(첫 번째 피연산자)를 cur의 맨 끝노드의 next로 추가
						cur = get_last_child(cur);

						if(new->child_head != NULL){
							new = new->child_head;

							new->parent->child_head = NULL;
							new->parent = NULL;
							new->prev = cur;
							cur->next = new;
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*"))
					{
						i = 0;

						// 다음 연산자를 찾아다님
						while(1)
						{
							if(!strcmp(tokens[*idx + i], ""))
								break;

							// ')' 가 아닌 연산자라면
							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0)
								break;

							i++;
						}
						
						// 다음 연산자가 우선순위가 더 높다면 현재의 연산자(new에 저장된 연산자)를 cur 맨 끝에 붙임
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name))
						{
							cur = get_last_child(cur);
							cur->next = new;
							new->prev = cur;
							cur = new;
						}
						else
						{
							cur = get_last_child(cur);

							if(new->child_head != NULL){
								new = new->child_head;

								new->parent->child_head = NULL;
								new->parent = NULL;
								new->prev = cur;
								cur->next = new;
							}
						}
					}
					else{
						cur = get_last_child(cur);
						cur->next = new;
						new->prev = cur;
						cur = new;
					}
				}
	
				else
				{
					cur = get_last_child(cur);

					cur->next = new;
					new->prev = cur;
	
					cur = new;
				}
			}
		}
		else if(is_operator(tokens[*idx]))
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx]))
					operator = cur;
		
				else
				{
					new = create_node(tokens[*idx], parentheses);
					operator = get_most_high_precedence_node(cur, new);

					// root
					if(operator->parent == NULL && operator->prev == NULL){
						// operator 보다 new의 우선순위가 낮다면 operator를 new의 아래에 배치
						if(get_precedence(operator->name) < get_precedence(new->name)){
							cur = insert_node(operator, new);
						}

						// operator 보다 new가 우선순위가 높고, operator의 child_head가 존재한다면 아랫쪽에 배치
						else if(get_precedence(operator->name) > get_precedence(new->name))
						{
							if(operator->child_head != NULL){
								operator = get_last_child(operator);
								cur = insert_node(operator, new);
							}
						}
						
						// 우선순위가 같은 경우
						// @TODO: new 노드 버리는데?
						else
						{
							operator = cur;
	
							// tokens[*idx]와 같은 연산자가 되거나 더이상 거슬러 올라갈 노드가 없을때까지 반복
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx]))
									break;
						
								if(operator->prev != NULL)
									operator = operator->prev;
								else
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0)
								operator = operator->parent;

							if(operator != NULL){
								if(!strcmp(operator->name, tokens[*idx]))
									cur = operator;
							}
						}
					}

					else
						cur = insert_node(operator, new);
				}

			}
			
			// ||, &&, |, &, +, * 가 아닌 연산자의 경우
			else
			{
				new = create_node(tokens[*idx], parentheses);

				// root 가 아직 없으면 루트로 설정
				if(cur == NULL)
					cur = new;
				else
				{
					operator = get_most_high_precedence_node(cur, new);

					if(operator->parentheses > new->parentheses)
						cur = insert_node(operator, new);

					// 루트
					else if(operator->parent == NULL && operator->prev ==  NULL) {
				
						// new 의 우선순위가 더 높은 상태
						if(get_precedence(operator->name) > get_precedence(new->name))
						{
							// operator에 다른 자식 노드가 있다면 operator를 operator의 맨 마지막 자식으로 이동하고, insert
							if(operator->child_head != NULL){
								operator = get_last_child(operator);
								cur = insert_node(operator, new);
							}
						}
				
						// new의 우선순위가 낮거나 같은 상태
						else
							cur = insert_node(operator, new);
					}
	
					else
						cur = insert_node(operator, new);
				}
			}
		}
		
		// 일반 알파벳, 숫자
		else
		{
			// 새로운 노드 생성
			new = create_node(tokens[*idx], parentheses);

			// 현재 노드가 없으면 새로 만들어진 노드가 현재 노드가 됨
			// 보통 ROOT 생성 시 이 과정이 필요
			if(cur == NULL)
				cur = new;

			// root는 아니면서 자식노드가 없는 경우 새로 만들어진 노드를 자식노드로 추가함
			else if(cur->child_head == NULL){
				cur->child_head = new;
				new->parent = cur;

				cur = new;
			}
			// root도 아니고 자식노드도 이미 있으면 맨 끝에 새로운 노드 추가
			else{
				cur = get_last_child(cur);

				cur->next = new;
				new->prev = cur;

				cur = new;
			}
		}

		*idx += 1;
	}

	return get_root(cur);
}

/**
 형제를 바꿔주는 함수
 @param parent 노드
 @return parent
 자식 노드를 원래 자식노드의 next로 지정하고 기존 자식노드는 새로운 자식노드의 next가 됨
 오른쪽 형제와 위치를 바꾼다고 생각하면 됨
 */
node *change_sibling(node *parent)
{
	node *tmp;
	
	tmp = parent->child_head;

	parent->child_head = parent->child_head->next;
	parent->child_head->parent = parent;
	parent->child_head->prev = NULL;

	parent->child_head->next = tmp;
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;
	parent->child_head->next->parent = NULL;		

	return parent;
}

/**
 노드 만들어주는 함수
 @param name 노드 이름(토큰 내용)
 @param parentheses 괄호 depth
 @return 만들어진 노드
 */
node *create_node(char *name, int parentheses)
{
	node *new;

	new = (node *)malloc(sizeof(node));
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(new->name, name);

	new->parentheses = parentheses;
	new->parent = NULL;
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new;
}

/**
 연산자의 우선순위를 구해주는 함수
 @param op 연산자
 @return 우선순위
		 0 '(', ')' 괄호이거나 연산자가 아닌경우
 */
int get_precedence(char *op)
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op))
			return operators[i].precedence;
	}
	return false;
}

/**
 op가 연산자인지 확인해주는 함수
 @param op 검사할 문자열
 @return true: 검사할 문자열 op가 연산자일때
		 false: 연산자가 아닐때
 */
int is_operator(char *op)
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){
			return true;
		}
	}

	return false;
}

void print(node *cur)
{
	if(cur->child_head != NULL){
		print(cur->child_head);
		printf("\n");
	}

	if(cur->next != NULL){
		print(cur->next);
		printf("\t");
	}
	printf("%s", cur->name);
}

/**
 상위 연산자를 찾아주는 함수
 @param cur 노드
 @return 연산자 노드
 */
node *get_operator(node *cur)
{
	if(cur == NULL)
		return cur;

	if(cur->prev != NULL)
		while(cur->prev != NULL)
			cur = cur->prev;

	return cur->parent;
}

/**
 트리의 루트를 찾아주는 함수
 @param cur 현재 위치에 해당하는 노드
 @return NULL인 node* : 트리 자체가 비어있는 경우
		 NULL이 아닌 node* : 부모 노드가 없는 최상위 노드
 */
node *get_root(node *cur)
{
	if(cur == NULL)
		return cur;

	while(cur->prev != NULL)
		cur = cur->prev;

	if(cur->parent != NULL)
		cur = get_root(cur->parent);

	return cur;
}

/**
 기존 부모 노드 중에서 new 노드보다 우선순위가 높은 노드를 찾아주는 함수
 @param cur 기존 부모 노드
 @param new 새로운 부모 노드 후보
 @return
 */
node *get_high_precedence_node(node *cur, node *new)
{
	// 루트 노드가 연산자이고, 새로 들어온 노드보다 우선순위가 높으면 루트 노드가 리턴
	if(is_operator(cur->name))
		if(get_precedence(cur->name) < get_precedence(new->name))
			return cur;

	// @TODO: ?? 대체 뭐지이건
	if(cur->prev != NULL){
		while(cur->prev != NULL){
			cur = cur->prev;
			
			// recursive하게 상위 노드를 탐색하며 우선순위가 높은 연산자를 찾는다.
			return get_high_precedence_node(cur, new);
		}

		// @TODO: 위에 while 문 때문에 이 if문은 절대 갈 수가 없는데..?
		if(cur->parent != NULL)
			return get_high_precedence_node(cur->parent, new);
	}

	if(cur->parent == NULL)
		return cur;
	
	// @TODO: 일단 대충 에러만 안나게 해보자
	return get_high_precedence_node(cur->parent, new);
}

/**
 cur 트리 중에서 우선순위가 가장 높은 연산자 노드를 찾아주는 함수
 @param cur 탐색 노드 중 최상위 노드
 @param new 새로운 연산자 노드
 @return 우선순위가 가장 높은 연산자 노드
 */
node *get_most_high_precedence_node(node *cur, node *new)
{
	node *operator = get_high_precedence_node(cur, new);
	node *saved_operator = operator;

	while(1)
	{
		if(saved_operator->parent == NULL)
			break;

		if(saved_operator->prev != NULL)
			operator = get_high_precedence_node(saved_operator->prev, new);

		else if(saved_operator->parent != NULL)
			operator = get_high_precedence_node(saved_operator->parent, new);

		saved_operator = operator;
	}
	
	return saved_operator;
}

/**
 기존 노드의 위치에 새로운 노드를 갈아끼고, 기존 노드를 새로운 노드의 child_head로 만들어주는 함수
 old의 위치에 new를 넣고 old가 new의 자식이 되는 거임
 @param old 기존 노드
 @param new 새로운 노드
 @return 새로운 노드의 포인터
 */
node *insert_node(node *old, node *new)
{
	if(old->prev != NULL){
		new->prev = old->prev;
		old->prev->next = new;
		old->prev = NULL;
	}

	new->child_head = old;
	old->parent = new;

	return new;
}

/**
 마지막 자식 노드를 찾아주는 함수
 @param cur 부모 노드
 @return 마지막 자식 노드
 */
node *get_last_child(node *cur)
{
	if(cur->child_head != NULL)
		cur = cur->child_head;

	while(cur->next != NULL)
		cur = cur->next;

	return cur;
}

/**
 형제 노드의 갯수를 세어주는 함수
 @param cur 노드
 @return cur의 형제노드의 갯수
 */
int get_sibling_cnt(node *cur)
{
	int i = 0;

	while(cur->prev != NULL)
		cur = cur->prev;

	while(cur->next != NULL){
		cur = cur->next;
		i++;
	}

	return i;
}

void free_node(node *cur)
{
	if(cur->child_head != NULL)
		free_node(cur->child_head);

	if(cur->next != NULL)
		free_node(cur->next);

	if(cur != NULL){
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL;
		free(cur);
	}
}

/**
 문자가 알파벳 혹은 숫자인지 판별해주는 함수
 @param c 입력 문자
 @return 1 문자가 알파벳 혹은 숫자인 경우
		 0 문자가 알파벳 혹은 숫자가 아닌 경우
 */
int is_character(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 타입 정의 문인지 확인하는 함수
 @param str 학생 답안 문자열
 @return	0 잘못된 포맷? 문자열 맨 앞에 공백이 두 개 이상인 경우
			2 gcc 명령어 또는 int, long 같은 선언문인 경우
			1 아무것도 아닌 경우
 */
int is_typeStatement(char *str)
{ 
	char *start;
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 
	
	start = str;
	strncpy(str2,str,strlen(str));
	remove_space(str2);

	while(start[0] == ' ')
		start += 1;

	// strstr : 첫 번째 인자 문자열 내에서 두 번째 인자 문자열을 검색한다.
	if(strstr(str2, "gcc") != NULL)
	{
		// start 의 처음 3글자가 gcc로 시작하는지를 확인
		strncpy(tmp2, start, strlen("gcc"));
		
		// gcc로 시작하지 않는 경우
		if(strcmp(tmp2,"gcc") != 0)
			return 0;
		
		// gcc로 시작하는 경우
		else
			return 2;
	}
	
	// 변수 타입 리스트 모두 검사
	for(i = 0; i < DATATYPE_SIZE; i++)
	{
		// 특정 변수 타입을 선언한게 발견
		if(strstr(str2,datatype[i]) != NULL)
		{
			// 공백을 다 제거한 str2에서 해당 변수 타입의 길이 만큼을 tmp로 복사
			strncpy(tmp, str2, strlen(datatype[i]));
			
			// 평범한 원본 문자열인 start 에서 해당 변수 타입의 길이 만큼을 tmp2로 복사
			strncpy(tmp2, start, strlen(datatype[i]));
			
			// 결국 학생 답안의 맨 앞에 변수 타입을 명시한 경우, 2리턴
			// 아닌 경우는 0리턴
			// 애초에 맨 앞에 변수 타입이 적힌것이 아니었다면 아무일도 없음 계속 반복문 돌게됨
			if(strcmp(tmp, datatype[i]) == 0)
				// 이 경우는 최초의 start가 "  int a;" 이런 꼴이였던거? 같음
				// 그래서 start[0] 이 공백일 때 한칸 띄웠음에도 공백이 남아있었다.
				if(strcmp(tmp, tmp2) != 0)
					return 0;
				else
					return 2;
		}
	}
	
	// 아무것도 아닌 경우
	return 1;

}

/**
 괄호에 둘러쌓인 타입 지정자를 찾아주는 함수 예) 형변환 long a = (long) b;
 @param tokens 토큰 테이블
 @return 1 이상의 정수 : 타입 지정자의 시작 토큰 인덱스
		 -1 : 없는 경우
 */
int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) 
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			// 0번째 토큰이 아닌 토큰에서 데이터 타입 문자열이 발견된 경우
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0)
			{
				// 그런 토큰의 직전 토큰이 '('이고 직후 토큰이 ')'이면서 다다음 토큰은 &, *, ), (, -, +, 문자 숫자 중 하나로 시작한다면 i를 리턴
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") 
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' 
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0])))  
					return i;
			}
		}
	}
	return -1;
}

/**
 struct 찾아주는 함수
 @param tokens 토큰 전체 테이블
 @return index struct 토큰 인덱스 넘버
		 -1 없는 경우
 */
int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) 
{
	int i, j;

   
	for(i = 0; i < TOKEN_CNT; i++)
	{
		// @TODO: 애초에 j를 쓰질 않는데 필요한 반복문인가?
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			// @TODO: (i + 1) <= TOKEN_CNT 가 맞나? i+1 < TOKEN_CNT 아님?
			if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1]))  
					return i;
		}
	}
	return -1;
}

/**
 모든 문자열이 *로 이루어졌는지 확인하는 함수
 @param str 검사 문자열
 @return 1 모든 문자열이 *로 이루어져있는 경우
		 0 하나라도 *이 아닌 문자가 있는 경우 또는 문자열의 길이가 0인 경우
 */
int all_star(char *str)
{
	int i;
	int length= strlen(str);
	
 	if(length == 0)	
		return 0;
	
	for(i = 0; i < length; i++)
		if(str[i] != '*')
			return 0;
	return 1;

}

int all_character(char *str)
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i]))
			return 1;
	return 0;
	
}

/**
 무의미한 괄호들을 없애주는 함수
 @param start 타입 지정자인 토큰 인덱스
 @param tokens 전체 토큰 테이블
 @return 0 잘못된 입력
		 1 제대로된 입력
 */
int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN])
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){
		// 타입 지정자 토큰이 struct인 경우 바로 아래 토큰을 이어 붙이고, 그 다음 토큰들을 한칸씩 위로 끌어올림
		if(!strcmp(tokens[start], "struct")) {		
			strcat(tokens[start], " ");
			strcat(tokens[start], tokens[start+1]);		 

			for(i = start + 1; i < TOKEN_CNT - 1; i++){
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		// "unsigned"이면서 그 다음 토큰은 ')'인 경우 "unsigned)" 에 ')' 토큰 다음 토큰까지 이어 붙인 뒤 ')'토큰부터 한 칸씩 위로 끌어올림
		// 즉 ')' 이거 다음 토큰은 token[start]와 token[start + 1] 두 개에 존재하게 됨
		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) {		
			strcat(tokens[start], " ");
			strcat(tokens[start], tokens[start + 1]);		 
			strcat(tokens[start], tokens[start + 2]);

			for(i = start + 1; i < TOKEN_CNT - 1; i++){
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		// start 이후에 등장한 모든 ')' 토큰을 넘김
		// 넘긴 횟수는 rcount 에 저장
	 		j = start + 1;		   
			while(!strcmp(tokens[j], ")")){
					rcount ++;
					if(j==TOKEN_CNT)
							break;
					j++;
			}
	
		j = start - 1;
		
		// start 이전에 등장한 모든 '(' 토큰을 넘김
		// 넘긴 횟수는 lcount 에 저장
		while(!strcmp(tokens[j], "(")){
					lcount ++;
					if(j == 0)
							break;
			   		j--;
		}
		
		// start 이전의 토큰 중 '('가 아닌 토큰이 숫자나 문자로 끝나는 경우 or j가 0인 경우
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0)
			lcount = rcount;

		// 좌우 괄호의 갯수가 다른 경우
		if(lcount != rcount )
			return false;

		// start 이전의 토큰 중 '('가 아닌 최초의 토큰이 sizeof 이면 true
		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){
			return true; 
		}
		
		// start 토큰이 unsigned 혹은 struct이면서 바로 다음 토큰이 ')'이 아니면 start 토큰과 start-lcount 토큰을 합치고 start 토큰 앞뒤로 있던 괄호들은 다 없앰
		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) {		
			strcat(tokens[start - lcount], tokens[start]);
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]);
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) {
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));
			}
		}
 		else{
			// 다다음 토큰이 '('로 시작하는 경우
			if(tokens[start + 2][0] == '('){
				j = start + 2;
				while(!strcmp(tokens[j], "(")){
					sub_lcount++;
					j++;
				} 	
				if(!strcmp(tokens[j + 1],")")){
					j = j + 1;
					while(!strcmp(tokens[j], ")")){
						sub_rcount++;
						j++;
					}
				}
				
				// 최소 하나의 ')'는 있어야 괄호 짝이 맞음
				else 
					return false;

				// 여는 괄호와 닫는 괄호의 갯수가 안맞음
				if(sub_lcount != sub_rcount)
					return false;
				
				// 무의미한 괄호들 사이에 유의미한 토큰이 하나 있음 그걸 start + 2로 끌어오고
				// 나머지는 다 0으로 초기화
				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]);	
				for(int i = start + 3; i<TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0]));

			}
			
			// start - lcount 과 start + rcount 사이에는 무수히 많은 무의미한 '('과 ')' 가 있고 그 중 유의미한 토큰은
			// start - lcount 번째에 있는 최초의 '('과
			// start + 1 번째에 있는 최초의 ')
			// start 번째에 있는 토큰
			// 이 세 토큰을 제외하고 다 없앤다.
			strcat(tokens[start - lcount], tokens[start]);
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) {
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0]));
			}
		}
	}
	return true;
}

/**
 토큰 배열을 비워주는 함수
 @param tokens 토큰 배열
 */
void clear_tokens(char tokens[TOKEN_CNT][MINLEN])
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));
}

/**
 오른쪽 공백을 지워주는 함수
 @param _str 원본 문자열
 @return 원본 문자열 내의 공백이 아닌 문자가 나오는 마지막 주소
*/
char *rtrim(char *_str)
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str);
	end = tmp + strlen(tmp) - 1;
	while(end != _str && isspace(*end))
		--end;

	*(end + 1) = '\0';
	_str = tmp;
	return _str;
}

/**
 왼쪽 공백을 지워주는 함수
 @param _str 원본 문자열
 @return 원본 문자열 내의 공백이 아닌 문자가 나오는 첫 주소
 */
char *ltrim(char *_str)
{
	char *start = _str;

	while(*start != '\0' && isspace(*start))
		++start;
	_str = start;
	return _str;
}

/**
 불필요한 공백은 없애주는 함수
 @param str 학생 답안 문자열(좌우 공백, ; 제거됨)
 @return 불필요한 공백이 없는 문자열을 리턴해준다.
  예) "#include	  <stdlib>" -> "#include <stdlib>
 */
char* remove_extraspace(char *str)
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	// 학생 답안 문자열에 "include<"가 포함되어있다면
	// "include <"로 바꿈
	if(strstr(str,"include<")!=NULL){
		start = str;
		end = strpbrk(str, "<");
		position = end - start;
	
		strncat(temp, str, position);
		strncat(temp, " ", 1);
		strncat(temp, str + position, strlen(str) - position + 1);

		str = temp;		
	}
	
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ')
		{
			if(i == 0 && str[0] ==' ')
				while(str[i + 1] == ' ')
					i++;	
			else{
				if(i > 0 && str[i - 1] != ' ')
					str2[strlen(str2)] = str[i];
				while(str[i + 1] == ' ')
					i++;
			} 
		}
		else
			str2[strlen(str2)] = str[i];
	}

	return str2;
}


/**
 공백을 제거해주는 함수
 @param str 원본 문자열, 이 문자열의 공백이 사라진다. 별도의 리턴이 없음
 */
void remove_space(char *str)
{
	char* i = str;
	char* j = str;
	
	while(*j != 0)
	{
		*i = *j++;
		if(*i != ' ')
			i++;
	}
	*i = 0;
}

/**
 괄호의 갯수를 파악하여 validation check
 @param	 str 검사할 학생 답안 문자열
 @return	0 여는 괄호의 갯수와 닫는 괄호의 갯수가 다를 때
			1 여는 괄호의 갯수와 닫는 괄호의 갯수가 같을 때
 @TODO: 갯수만 파악해서 정말 validation check가 가능한가? 예를 들어 ")a += 10(" 이런거 ㅇㅇ
 */
int check_brackets(char *str)
{
	char *start = str;
	int lcount = 0, rcount = 0;
	
	while(1){
		// strpbrk : 첫 번째 인자로 주어진 문자열 내에 두 번째 인자로 주어진 문자열의 문자들을 검색하여
		// 찾는다면 왼쪽에서부터 그 주소를 리턴한다.
		// 주의 : "()" 를 찾는 것이 아니라 '(', 와 ')' 를 찾는 것이다.
		if((start = strpbrk(start, "()")) != NULL){
			if(*(start) == '(')
				lcount++;
			else
				rcount++;

			// += 1을 안해주면 무한루프
			start += 1;
		}
		else
			break;
	}

	// 여는 괄호의 갯수와 닫는 괄호의 갯수가 다르면 0리턴
	if(lcount != rcount)
		return 0;
	else 
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN])
{
	int i;
	
	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], ""))
			break;

	return i;
}
