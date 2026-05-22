CC = g++
CFLAGS = -std=c++20 -O3 -g -Wall -Wextra -Werror
all: consumer producer benchmark
	@echo "We compile the consumer & producer & benchmark!"
	
producer: producer.c spsc_queue.cpp
	$(CC) $(CFLAGS) -o producer producer.c spsc_queue.cpp

consumer: consumer.c spsc_queue.cpp
	$(CC) $(CFLAGS) -o consumer consumer.c spsc_queue.cpp

benchmark: benchmark.c spsc_queue.cpp
	$(CC) $(CFLAGS) -o benchmark benchmark.c spsc_queue.cpp -pthread

.PHONY: clean
clean:
	rm -f benchmark producer consumer *.o *.s
