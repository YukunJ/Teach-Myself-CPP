CC = gcc
CFLAGS = -O3 -g -Wall -Wextra -Werror -std=c11

all: consumer producer benchmark
	@echo "We compile the consumer & producer & benchmark!"
	
producer: producer.c spsc_queue.c
	$(CC) $(CFLAGS) -o producer producer.c spsc_queue.c

consumer: consumer.c spsc_queue.c
	$(CC) $(CFLAGS) -o consumer consumer.c spsc_queue.c

benchmark: benchmark.c spsc_queue.c
	$(CC) $(CFLAGS) -o benchmark benchmark.c spsc_queue.c -pthread

.PHONY: clean
clean:
	rm -f benchmark producer consumer *.o *.s
