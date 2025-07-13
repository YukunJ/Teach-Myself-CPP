### Design a Shared-Memory-Queue IPC

In this tutorial, we will design a shared-memory queue to support inter-process commmunication on the same localhost.

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