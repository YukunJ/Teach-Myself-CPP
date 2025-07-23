### Design a Shared-Memory-Queue IPC

In this tutorial, we will design a single-producer-multiple-consumer(SPMC) shared-memory queue to support inter-process commmunication on the same localhost.

+ Day 1

Build a producer that create a shared-memory queue and write a "Hello World" string message into it. Build a consumer that connects to this queue and grab that string message.

With no queue protocol design, no synchronization, no error-checking, just with bare-hand and look-up documentation from man page, we made it the first working version.

```shell
$ ./producer
Producer creating the shared-memory queue /queue...
Producer created the shared-memory queue /queue of size 100 and mapped it into memory.
Producer sent a msg "Hello from producer" into the queue.
Producer closed the queue after 5 seconds and exit...
```

```shell
$ ./consumer
Consumer attaching to the shared-memory queue /queue...
Consumer attached to the shared-memory queue /queue.
Consumer read a msg from queue: "Hello from producer"
Consumer closed the queue and exit...
```

+ Day 2

We will wrap the connection semantics into a reuseable APIs and define a basic header/protocol for the queue so that producer and consumer could agree on the message size.

Currently the queue is bounded and does not support wrap around, so the queue size is limited. And the data field is not cache-line aligned to avoid false-sharing. 

And it only support 1 producer 1 consumer, and assume the producer queue is created before the consumer connected to it. And the producer will wait for consumer to be connected before it starts to publish.

```shell
$ ./producer
creating spmc_queue /spmc_test_queue of size 100 and capacity 10 with mode writer
spmc_queue created
destroying spmc_queue /spmc_test_queue of mode writer
spmc_queue destroyed
```

```shell
$ ./consumer 
creating spmc_queue /spmc_test_queue of size 100 and capacity 10 with mode reader
spmc_queue created
Received message from producer: Hello from spmc_queue producer!
destroying spmc_queue /spmc_test_queue of mode reader
spmc_queue destroyed
```

+ Day 3

We add the support to allow the data buffer to wrap around as a ring buffer. Since both the writer and reader will need to know each other's index to decide if they could enqueue/dequeue, the indexes need to be thread-safe. we use the C11 `<stdatmoic.h>` atomic operations, which are cheaper than mutex.

And also we add a basic performance testing benchmark for future calibration. Currently the code we had is performing at `923.601 MB/s` throughput.

```shell
$ ./benchmark 
Initializing the performance benchmark...
creating spmc_queue /spmc_benchmark_queue of size 64 and capacity 1024 with mode writer
spmc_queue created
creating spmc_queue /spmc_benchmark_queue of size 64 and capacity 1024 with mode reader
spmc_queue created
Initialized performance benchmark
waiting for the producer & consumer thread to be ready...
producer thread spawns
consumer thread spawns
performance benchmark starts
performance benchmark ends
Elapsed time: 1.109 seconds
Throughput: 923.601 MB/s
test_producer_sum = 33563254 and test_consumer_sum = 33563254
Destroying the performance benchmark...
destroying spmc_queue /spmc_benchmark_queue of mode writer
spmc_queue destroyed
destroying spmc_queue /spmc_benchmark_queue of mode reader
spmc_queue destroyed
Destroyed performance benchmark
```