#Program Name: makefile

#Author: David Eaton

#Date: 5/19/20

#Program Description: This program is a makefile that compiles the files for program 3.
#Citation: The overall structure was inspired by the class module make help: "Introduction to Makefiles: How to Create a Simple Makefile."
#(https://oregonstate.instructure.com/courses/1719543/pages/make-help?module_item_id=18712386)

smallsh: smallsh.o smallsh_builtins.o
	gcc smallsh.o smallsh_builtins.o -o smallsh

smallsh.o: smallsh.c
	gcc -c smallsh.c

smallsh_builtins.o: smallsh_builtins.c smallsh_builtins.h
	gcc -c smallsh_builtins.c

clean:
	rm *.o  smallsh