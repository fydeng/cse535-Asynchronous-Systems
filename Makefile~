CC = g++

FLAGS = -g -O2 

all: client server 

server: server.cpp
	${CC} ${FLAGS} -o server server.cpp

client: client.cpp
	${CC} ${FLAGS} -o client client.cpp

clean:
	rm server client
