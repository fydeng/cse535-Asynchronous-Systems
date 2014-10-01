CC = g++

LIBS = -lresolv -lnsl -lpthread\
		/home/fydeng/unpv13e/libunp.a\
		
FLAGS = -g -O2 

CFLAGS = ${FLAGS} -I/home/fydeng/unpv13e/lib

all: server master client

server: server.o readline.o
	${CC} ${FLAGS} -o server server.o readline.o ${LIBS}

server.o: server.cpp 
	${CC} ${CFLAGS} -c server.cpp

master: master.o readline.o
	${CC} ${FLAGS} -o master master.o readline.o ${LIBS}

master.o: master.cpp
	${CC} ${CFLAGS} -c master.cpp

client: client.o
	${CC} ${FLAGS} -o client client.o ${LIBS}

client.o: client.cpp
	${CC} ${CFLAGS} -c client.cpp

#using the thread-safe version of readline.c from directory "threads"

readline.o: /home/fydeng/unpv13e/threads/readline.c
	gcc ${CFLAGS} -c /home/fydeng/unpv13e/threads/readline.c

clean:
	rm client.o server.o master.o master server client readline.o