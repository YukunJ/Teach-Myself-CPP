CC = gcc
CFLAGS = -O3 -g -Wall -Werror -std=c11

all: consumer producer benchmark
	@echo "We compile the consumer & producer & benchmark!"
	
producer: producer.c spmc_queue.c
	$(CC) $(CFLAGS) -o producer producer.c spmc_queue.c

consumer: consumer.c spmc_queue.c
	$(CC) $(CFLAGS) -o consumer consumer.c spmc_queue.c

benchmark: benchmark.c spmc_queue.c
	$(CC) $(CFLAGS) -o benchmark benchmark.c spmc_queue.c -pthread

.PHONY: clean
clean:
	rm -f producer consumer *.o *.s
