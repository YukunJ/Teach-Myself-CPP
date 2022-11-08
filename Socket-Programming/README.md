### Socket Network Programming 

In this section, we will first go over some key concepts about **socket network programming** on Linux/Unix. Socket programming is essentially the backbone of network communication supported by the operating system. Especially on C/C++, if you dig deep down into any network libraries, they all boil down to the single key thing -- **Socket**.

Our main reference is the open source book <[Network Programming Using Internet Sockets](https://beej.us/guide/bgnet/)> by Beej.

---
#### What is a Socket?

In plain definition, **Socket** is the way to speak to other programs using standard Unix file descriptors.

Unix Programs do any sort of I/O by reading/writing to a file descriptor, whose underlying could be a network connection, a FIFO pipe, a real on-the-disk text file, etc.

There are two types of Internet Sockets: `SOCK_STREAM` and `SOCK_DGRAM`. `SOCK_STREAM` corresponds to the TCP protocol that ensures reliable bidirectional byte stream transfer. `SOCK_DGRAM` is the UDP protocol that's faster and unreliable delivery.

A packet is transferred over the Internet with many layers' encapsulation. The below is an example:

![avatar](./pics/data_encapsulation.png)

In theory, there is a 7-layered OSI network model. However, a more relevant one here is like this:

+ Application Layer (*telnet*, *ftp*, etc.)
+ Host-to-Host Transport Layer (*TCP*, *UDP*)
+ Internet Layer (*IP* and *routing*)
+ Network Access Layer (*Ethernet*, *wi-fi*, etc.)

In simple word, all we need to do is build up the data we want to send for the Application layer, and stuff it into the stream sockets via `send()`. The operating system kernel builds the Transport layer and Internet layer on for us, and the hardware does the Network Access layer.

---
#### IP Addresses, *structs*, and Data Munging

There are two kinds of **IP Addresses**: IPv4 and IPv6. IPv4 is made up of 32 bits, for example `192.0.2.111`. IPv6 is created because we are running short of IPv4 addresses. IPv6 address is 128 bits, `2^96` times more available addresses than IPv4 has.

**Subnet** is the way for organizations to split up the "organizational indicator" and "host indicator". For example, with IPv4, `192.0.2.12` could mean that, the first 3 bytes `192.0.2.00` is the organization indicator, and this leaves the host 1 bytes. Namely there could be `2^8` hosts in this organization. The host is `12` in this case.

**Port Number** is a 16-bit number that helps multi-plexing. Think of the IP address as the street address of a hotel, and the port number is the room number of the hotel. It always the network to distinguish between different services running on the same host machine.