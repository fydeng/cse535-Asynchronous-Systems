#!/bin/sh
SERVER='./server'
CLIENT='./client'
MASTER='./master'
LOGS='./logs'

killall SERVER
killall CLIENT
killall MASTER
./MASTER $1 > LOGS/master_log &
for ((i = 1; i <= 6; i++));
do
	./SERVER $i $1 > LOGS/server_log$i &
done
./client $1 > LOGS/client_log
killall SERVER
killall MASTER
