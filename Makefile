CC = g++

FLAGS = -lboost_system-mt -g -O2 

all: client server master

server: server.cpp
	${CC} ${FLAGS} -o server server.cpp

master: master.cpp
	${CC} ${FLAGS} -o master master.cpp

client: client.cpp
	${CC} ${FLAGS} -o client client.cpp

clean:
	rm master server client
