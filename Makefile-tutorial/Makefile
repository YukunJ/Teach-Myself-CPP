# MakeFile for this small number guess program
# to illustrate how to make a simple MakeFile
# to automate the compiling process

# Compiler, g++ for C++ program and gcc for C program
CC = g++

#   We provide the following Flags
# 	optimization level 0
# 	debugger
# 	turn all optional warnings
# 	turn all warnings into errors
# 	set standard c++ to c11
CFLAGS = -O0 -g -Wall -Werror -std=c++11

# default target when we only type "make"
all: guesser
	@echo "We compile the guesser!"

# the followings are step-by-step compilation process
# actually we can do that in one step, see further below

# create adder.s file from adder.cpp and adder.h
adder.s: adder.cpp
	$(CC) $(CFLAGS) -S adder.cpp -o adder.s

# create guesser.s file from guesser.cpp
guesser.s: guesser.cpp
	$(CC) $(CFLAGS) -S guesser.cpp -o guesser.s
	
# create adder.o file from adder.s
adder.o: adder.s
	$(CC) $(CFLAGS) -c adder.s -o adder.o

# create guesser.o file from guesser.s
guesser.o: guesser.s
	$(CC) $(CFLAGS) -c guesser.s -o guesser.o

# create the step-by-step final executable guesser-stepwise
guesser-stepwise: guesser.o adder.o
	$(CC) $(CFLAGS) -o guesser-stepwise guesser.o adder.o
	
# Concise one-step compilation rule
# it combines all the assembly + compiler step we did above
guesser: adder.cpp guesser.cpp adder.h
	$(CC) $(CFLAGS) -o guesser adder.cpp guesser.cpp

# clean up command
.PHONY: clean
clean:
	# remove all the files with extension .o or .s and the guesser executable
	rm -f guesser *.o *.s
