#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

char buffer1[1024 * 1024];
char buffer2[1024 * 1024];
char questionList[120][10];
char idList[10][10];
int wrongList[10][120];
int wrongCountList[10];

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s)); 
}

int main() {
	char ansDirName[10] = "ANS_DIR";
	DIR* ansDir = opendir(ansDirName);
	struct dirent* ansDirent;
	int questionCount = 0;

	while(( ansDirent = readdir(ansDir)) != NULL) {
		if (strcmp(strrchr(ansDirent->d_name, '.'), ".txt") && strcmp(strrchr(ansDirent->d_name, '.'), ".stdout"))
			continue;

		strcpy(questionList[questionCount++], ansDirent->d_name);
	}

	qsort(questionList, questionCount, sizeof(questionList[0]), strcmp);

	char stdDirName[10] = "STD_DIR2";
	DIR* stdDir = opendir(stdDirName);
	struct dirent* stdDirent;
	int idCount = 0;

	while ((stdDirent = readdir(stdDir)) != NULL) {
		if (!strcmp(stdDirent->d_name, ".") || !strcmp(stdDirent->d_name, "..") || strncmp(stdDirent->d_name, "20", 2))
			continue;

		strcpy(idList[idCount++], stdDirent->d_name);
	}
	qsort(idList, idCount, sizeof(idList[0]), strcmp);

	for (int i = 0; i < questionCount; i++) {
		char stdfname[512];
		char ansfname[512];
		sprintf(ansfname, "%s/%s", ansDirName, questionList[i]);

		int ansFile = open(ansfname, O_RDONLY);
		int ansFileSize = lseek(ansFile, 0, SEEK_END);
		lseek(ansFile, 0, SEEK_SET);
		memset(buffer1, 0, sizeof(buffer1));
		read(ansFile, buffer1, ansFileSize);

		for (int j = 0; j < idCount; j++) {
			sprintf(stdfname, "%s/%s/%s", stdDirName, idList[j], questionList[i]);
			int stdFile = open(stdfname, O_RDONLY);
			if (stdFile < 0) {
				fprintf(stderr, "%s cannot open\n", stdfname);
				wrongList[j][wrongCountList[j]++] = i;
				continue;
			}
			int stdFileSize = lseek(stdFile, 0, SEEK_END);
			lseek(stdFile, 0, SEEK_SET);
			memset(buffer2, 0, sizeof(buffer2));
			read(stdFile, buffer2, stdFileSize);

			char* s = strstr(buffer1, trim(buffer2));
			if (s == NULL && strcmp(buffer1, trim(buffer2))) {
				wrongList[j][wrongCountList[j]++] = i;
			} else if (s != NULL) {
				s += strlen(trim(buffer2));
				while (1) {
					if (*s == '\0' || *s == ':')
						break;

					if (*s != ' ' && *s != '\n' && *s != '\t') {
						wrongList[j][wrongCountList[j]++] = i;
						break;
					}
					s++;
				}
			}

			close(stdFile);
		}
		close(ansFile);
	}

	for (int i = 0; i < idCount; i++) {
		printf("%s 의 오답리스트:\n", idList[i]);
		for (int j = 0; j < wrongCountList[i]; j++) {
			printf("%s, ", questionList[wrongList[i][j]]);
		}
		printf("\n");
	}
	return 0;
}
