# build an executable named lets_talk from lets_talk.c
all: lets_talk.c
	gcc -g -Wall -pthread -o lets_talk lets_talk.c list.c

clean:
	$(RM) lets_talk

valgrind: 
	valgrind --leak-check=full ./lets_talk 3000 localhost 3001
