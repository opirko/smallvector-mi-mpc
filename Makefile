.PHONY: clean

INC=-I/src
CC = g++
COPT = -std=c++14 -Wall -pedantic -g


all: test

test: src/smallVector.hpp testMain.cpp
	$(CC) ${COPT} testMain.cpp -o testMain

clean:
	rm testMain
