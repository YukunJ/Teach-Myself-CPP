CC = gcc
CFLAGS = -O3 -g -Wall -Werror -std=c11

all: consumer producer
	@echo "We compile the consumer & producer!"
	
producer: producer.c spmc_queue.c
	$(CC) $(CFLAGS) -o producer producer.c spmc_queue.c

consumer: consumer.c spmc_queue.c
	$(CC) $(CFLAGS) -o consumer consumer.c spmc_queue.c

.PHONY: clean
clean:
	rm -f producer consumer *.o *.s
