# Name: Aaron Colin Foote
# User: acf502 / 4258770
# Last Modified Date: 31st October 2013
# File Description: Makefile for Assignment 4 of CSCI203
matching:	A4.o
	CC -o matching A4.o
A4.o:	A4.cpp A4.h
	CC -c A4.cpp
clean:
	rm -rf *.o
