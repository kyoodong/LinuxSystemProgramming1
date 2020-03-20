ssu_score : main.o ssu_score.o blank.o
	gcc main.o ssu_score.o blank.o -o ssu_score

main.o : main.c ssu_score.h
	gcc -c main.c

ssu_score.o : ssu_score.c ssu_score.h blank.h
	gcc -c ssu_score.c

blank.o : blank.c blank.h
	gcc -c blank.c

debug : main_debug.o ssu_score_debug.o blank_debug.o
	gcc main_debug.o ssu_score_debug.o blank_debug.o -o ssu_score_debug -g

main_debug.o : main.c ssu_score.h
	gcc -c main.c -o main_debug.o -g

ssu_score_debug.o : ssu_score.c ssu_score.h blank.h
	gcc -c ssu_score.c -o ssu_score_debug.o -g

blank_debug.o : blank.c blank.h
	gcc -c blank.c -o blank_debug.o -g

clean :
	rm *.o
	rm ssu_score
