CC = g++
CFLAGS = -std=c++11

all: math-lib

math-lib: src/main.cpp src/math.cpp src/math.h
	$(CC) $(CFLAGS) -o math-lib src/main.cpp src/math.cpp

.PHONY: clean
clean:
	rm -f math-lib *.o *.s
