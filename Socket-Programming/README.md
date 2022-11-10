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

**Byte Order** might differ by different machine architectures, i.e. Little Endian or Big Endian. To ensure the portability and comptability of our program, we can use convert functions:

| **Function** | **Description**       |
|--------------|-----------------------|
| `htons()`     | host to network short |
| `htonl()`     | host to network long  |
| `ntohs()`      | network to host short |
| `ntohl()`      | network to host long  |

A **socket descriptor** is just a regular `int` type.

A `struct addrinfo` is used to prepare and store the socket address structures for subsequent use.

```C
struct addrinfo {
  int 			ai_flags;	// AI_PASSIVE, AI_CANONNAME, etc.
  int 			ai_family;	// AF_INET, AF_INET6, AF_UNSPEC
  int 			ai_socktype;	// SOCK_STREAM, SOCK_DGRAM
  int 			ai_protocol;	// use 0 for "any"
  size_t 		ai_addrlen;	// size of ai_addr in bytes
  struct sockaddr 	*ai_addr;	// struct sockaddr_in or _in6
  char 			*ai_canonname;	// full canonical hostname

  struct addrinfo 	*ai_next;	// linked list, next node
};
```

We can load this up by calling `getaddinfo()`, which returns a linked list of such structs. 

We see the `ai_addr` is a pointer to a `struct sockaddr`, which stores either IPv4 or IPv6 socket address information. Its generally format is as follows:

```C
struct sockaddr {
  unsigned short	sa_family;	// address family, AF_INET or AF_INET6
  char			sa_data[14];	// 14 bytes of protocol address
};
```

When we know this is an IPv4 sock address, we can cast this `struct sockaddr *` to `struct sockaddr_in *`, whose 16 bytes layout is as follows:

```C
// IPv4 only
struct sockaddr_in {
  short int		sin_family;	// Address family, AF_INET
  unsigned short int 	sin_port;	// Port number
  struct in_addr	sin_addr;	// Internet address
  unsigned char		sin_zero[8];	// Padding
};
```

Notice the `sin_port` here must be in network byte order by using `htons()`. And the `struct in_addr sin_addr` is just a finaly wrapper for the 4 bytes IPv4 address.

```C
struct in_addr {
  uint32_t s_addr;	// 4 Bytes IPv4 address in network order
};
```

On the other hand, if the address family is `AF_INET6` IPv6, a similar structure exists. We can cast `struct sockaddr *` to `struct sockaddr_in6 *` with layout:

```C
// IPv6 only
struct sockaddr_in6 {
  u_int16_t		sin6_family;	// Address family, AF_INET6
  u_int16_t 		sin6_port;	// Port number
  u_int32_t		sin6_flowinfo;	// IPv6 flow information
  struct in6_addr	sin6_addr;	// IPv6 address
  u_int32_t		sin6_scope_id;	// Scope ID
};

// the final wrapper for 16 bytes IPv6 address
struct in6_addr {
  unsigned char 	s6_addr[16];	// IPv6 address
};
```

It would be tedious to fill out the string representation of an IP Address into such struct and vice verse. Therefore there are utility functions to help us do so.

Firstly, `inet_pton()` convert a string IP address into the network representation:

```C
struct sockaddr_in sa;	// IPv4
struct sockaddr_in6 sa6;// IPv6

inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr));		// IPv4
inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr));	// IPv6
```

Conversely, `inet_ntop()` helps to convert the IP Address back to string representation.

```C
// IPv4
char ip4[INET_ADDRSTRLEN];
struct sockaddr_in sa;

inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
printf("The IPv4 address is: %s\n", ip4);

// IPv6
char ip6[INET6_ADDRSTRLEN];
struct sockaddr_in6 sa6;

inet_pton(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
printf("The IPv6 address is: %s\n", ip6);
```

---

#### System Calls or Bust

1. `getaddrinfo()`

we will use this function to do the DNS lookups and fill in the necessary socket struct information for us automatically. 

```C
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node,	// e.g. "www.example.com" or IP
		const char *service,	// e.g. "http" or port number
		const struct addrinfo *hints,
		struct addrinfo **res);
```

The `node` parameter is the host name or an IP address to connect to. The 	`service` parameter could be a well-knwn service like "http" or "ftp" or just the direct port number. The `hints` parameter points to a `struct addrinfo` that you have already filled out with relevant requirements. And then the function will populate the `res` with the linked list of information.

For example, if you're a server that listen on your own host's IP address with port 3490:

```C
int status;
struct addrinfo hints;
struct addrinfo *servinfo;		// will point to the results

memset(&hints, 0, sizeof hints);	// make sure the struct is empty
hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
hints.ai_flags = AI_PASSIVE;		// fill in my IP for me

if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
  fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
  exit(1);
}

...

freeaddrinfo(servinfo);	// free the linked list
```

Similarly, if you're a client wishing to connect ot "www.example.net" on port 3490:

```C
int status;
struct addrinfo hints;
struct addrinfo *servinfo;		// will point to the results

memset(&hints, 0, sizeof hints);	// make sure the struct is empty
hints.ai_family = AF_UNSPEC;		// dont' care IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets

// get ready to connect
status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);

...

freeaddrinfo(servinfo);	// free the linked list
```

[Here](./sample_src/showip.c) is a complete sample program to show the socket information given a host name for reference.

2. `socket()`

This function allow you to say what kind of socket you want in term of IPv4 or IPv6, stream or datagram, and TCP or UDP.

```C
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

`domain` is either `PF_INET` or `PF_INET6`, it's fine to just pass in `AF_INET` or `AF_INET6`. `type` is `SOCK_STREAM` or `SOCK_DGRAM`, and `protocol` can be set to `0` to choose the proper protocol for the given type, or you can call the function `getprotobyname()` to lookup the protocol you want, either "tcp" or "udp".

In most of cases, what we should really need is not to hardcode, but rather pass in the information got from `getaddrinfo()` function:

```C
int s;
struct addrinfo hints, *res;

// do the loop up
// assume we have already filled in "hints" struct
getaddrinfo("www.example.com", "http", &hints, &res);

s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```

The call to `socket()` returns an integer as the **socket descriptor**, or -1 on error. The global variable `errno` is set to the error's value if it happens. We definitely would need to do more error-checking there.

3. `bind()`

Once we have a socker descriptor, we might want to associate it with a port on the local machine. This is necessary for we're a server and is about to call `listen()` for incoming connections on a specific port. If we're a client who is about to do `connect()`, this step is probably unnecessary.

```C
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

`sockfd` is the socket descriptor returned by `socket()`. `my_addr` is a pointer to `struct sockaddr` that contains our port and IP Address. `addrlen` is the length in bytes of that address.

Let's look at an example that binds the socket to the host on port 3490:

```C
struct addrinfo hints, *res;
int sockfd;

// first look up address structs with getaddrinfo()
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;		// use IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;		// fill in my IP for me

getaddrinfo(NULL, "3490", &hints, &res);

// make a socket
sockfd = socket(res->ai_family, res->ai_socktyoe, res->ai_protocol);

// bind it to the port we passed in to getaddrinfo()
bind(sockfd, res->ai_addr, res->ai_addrlen);
```

One thing to watchout when calling `bind()` is that, all ports below 1024 are **RESERVED** for the kernel. As a user, you may choose between 1024 to 65535. And sometimes, when you want to rerun the same program again to listen on a same port, the `bind()` might fail and tell you that "Address already in use". It's because the socket that was connected might still be hanging around in the kernel. You may call `setsockopt()` to allow reusage of the same port.

And be sure to check the return status of `bind()`, where `errno` might be set.

4. `connect()`

`connect()` gives the functionality to associate a socket descriptor to a remote listener, assuming we're the client role.

```C
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
```

The parameters are easy to understand. `sockfd` is the socket descriptor we want to associaed it with. `serv_addr` is pointer to `struct sockaddr` which stores the destination IP and port number, with `addrlen` being the length in bytes of the destination address struct.

Let's see an example where we make a socket connection to "www.example.com" on port 3490:

```C
struct addrinfo hints, *res;
int sockfd;

// first, look up destination address struct
memeset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;

getaddrinfo("www.example.com", "3490", &hints, &res);

// make a socket
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// connnect
connect(sockfd, res->ai_addr, res->ai_addrlen);
```

Notice here, as a client side, once we get the socket descriptor, we directly jump to `connect()` instead of calling `bind()` first. Because we only care where we're going (the remote port). The kernel will choose a local port for us.

5. `listen()`