#!/bin/sh
SERVER='./server'
CLIENT='./client'
LOGS='./logs'

killall SERVER
killall CLIENT
for ((i = 1; i < 4; i++));
do
	./SERVER $i $1 & #> LOGS/server_log$i &
done
#./client $1 > LOGS/client_log
#killall SERVER
