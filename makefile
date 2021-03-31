#Program Name: makefile

#Author: David Eaton

#Date: 5/19/20

#Program Description: This program is a makefile that compiles the files for shellLite.
#Citation: The overall structure was inspired by the class module make help: "Introduction to Makefiles: How to Create a Simple Makefile."
#(https://oregonstate.instructure.com/courses/1719543/pages/make-help?module_item_id=18712386)

shellLite: shell_lite.o shell_lite_builtins.o
	gcc shell_lite.o shell_lite_builtins.o -o shellLite

shell_lite.o: shell_lite.c
	gcc -c shell_lite.c

shell_lite_builtins.o: shell_lite_builtins.c shell_lite_builtins.h
	gcc -c shell_lite_builtins.c

clean:
	rm *.o  shellLite
