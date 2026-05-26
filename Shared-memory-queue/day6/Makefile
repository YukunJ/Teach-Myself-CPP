CC = g++
CFLAGS = -std=c++23 -O3 -g -Wall -Wextra -Werror -Wno-interference-size
all: consumer producer benchmark
	@echo "We compile the consumer & producer & benchmark!"
	
producer: producer.cpp spsc_queue.cpp
	$(CC) $(CFLAGS) -o producer producer.cpp spsc_queue.cpp

consumer: consumer.cpp spsc_queue.cpp
	$(CC) $(CFLAGS) -o consumer consumer.cpp spsc_queue.cpp

benchmark: benchmark.cpp spsc_queue.cpp
	$(CC) $(CFLAGS) -o benchmark benchmark.cpp spsc_queue.cpp -pthread

.PHONY: clean
clean:
	rm -f benchmark producer consumer *.o *.s
