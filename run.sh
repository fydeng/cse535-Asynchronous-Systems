#!/bin/sh
SERVER='./server'
CLIENT='./client'

./CLIENT

for ((i = 1; i < 10; i++));
do
	./SERVER $i 
done
