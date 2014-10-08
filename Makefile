CC = g++

LIBS = -lresolv -lnsl -lpthread\
		../unpv13e/libunp.a\
		
FLAGS = -g -w -O2 

CFLAGS = ${FLAGS} -I../unpv13e/lib

all: client server 

server: server.o readline.o
	${CC} ${FLAGS} -o server server.o readline.o ${LIBS}

server.o: server.cpp 
	${CC} ${CFLAGS} -c server.cpp

client: client.o
	${CC} ${FLAGS} -o client client.o ${LIBS}

client.o: client.cpp
	${CC} ${CFLAGS} -c client.cpp

#using the thread-safe version of readline.c from directory "threads"

readline.o: ../unpv13e/threads/readline.c
	gcc ${CFLAGS} -c ../unpv13e/threads/readline.c

clean:
	rm client.o server.o server client readline.o
