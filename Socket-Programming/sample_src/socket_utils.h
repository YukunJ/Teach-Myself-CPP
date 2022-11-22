/**
 * @file socket_utils.h
 * @author Yukun J
 * @expectation this header file should be compatible to compile with C and C++
 * program on Linux
 * @init_date Nov 13 2022
 * @reference1 <Beej's Guide to Network Programming>
 * @reference2 CMU 15213 course's proxy-lab starter code
 *
 * This is a header file implementing several utility socket functions
 * Many of them are adapted from the online open-source book as in @reference1
 * and also CMU course project's provided helper functions as in @reference2
 */

#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INIT_POLL_ARRAY_CAPACITY 8

/**
 * @brief Get the real struct address pointer position.
 * It discriminates between IPv4 or IPv6 by inspecting the sa_family field.
 * @param sa pointer to the general struct sockaddr
 * @return void* pointer to struct sockaddr_in (IPv4) or sockaddr_in6 (IPv6)
 */
void *get_addr_in(const struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    // IPv4
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  // IPv4
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/**
 * @brief Build a client side socket.
 * Caller should close the socket descriptor after usage.
 * @param hostname the server's IP Address or Hostname to connect to
 * @param port the server's listening port to connect to
 * @param verbose if set True, intermediate logging will be made to stdout
 * @return the client socket descriptor, or -1 if any error happens
 */
int build_client(const char *host, const char *port, bool verbose) {
  int error;
  int client_fd;
  struct addrinfo hints;
  struct addrinfo *server_info;
  struct addrinfo *walker;

  /* provide hints about what type of socket we want */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;      // either IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // want TCP Stream socket

  if ((error = getaddrinfo(host, port, &hints, &server_info)) != 0) {
    if (verbose) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    }
    return -1;
  }

  /* walk through the linked list to connect to the first succeeds */
  for (walker = server_info; walker != NULL; walker = walker->ai_next) {
    if ((client_fd = socket(walker->ai_family, walker->ai_socktype,
                            walker->ai_protocol)) == -1) {
      if (verbose) {
        perror("client: socket()");
      }
      continue;
    }

    if (connect(client_fd, walker->ai_addr, walker->ai_addrlen) == -1) {
      close(client_fd);
      if (verbose) {
        perror("client: connect()");
      }
      continue;
    }

    break;  // succeed in building a client socket
  }

  if (walker == NULL) {
    if (verbose) {
      perror("client: fail to connect to any");
    }
    freeaddrinfo(server_info);  // done with this linked list
    return -1;
  }

  if (verbose) {
    char server[INET6_ADDRSTRLEN];
    inet_ntop(walker->ai_family, get_addr_in(walker->ai_addr), server,
              sizeof server);
    fprintf(stdout, "client: connected with address [%s] on port [%s]\n",
            server, port);
  }

  freeaddrinfo(server_info);  // done with this linked list

  return client_fd;
}

/**
 * @brief Build a server slide socket.
 * It assume the server will listen on its own local machine's IP Address
 * Caller should close the socket descriptor after usage.
 * @param port the port server will be listening to
 * @param backlog how many pending connections to be accept()-ed the server
 * queue will hold
 * @param verbose if set True, intermediate logging will be made to stdout
 * @return int the listening socket descriptor, -1 if any error happens
 */
int build_server(const char *port, const int backlog, bool verbose) {
  int yes = 1;
  int error;
  int server_fd;
  struct addrinfo hints;
  struct addrinfo *server_info;
  struct addrinfo *walker;

  /* provide hints about what type of socket we want */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;      // either IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // want TCP Stream socket
  hints.ai_flags = AI_PASSIVE;      // use my own IP Address

  if ((error = getaddrinfo(NULL, port, &hints, &server_info)) != 0) {
    if (verbose) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    }
    return -1;
  }

  /* walk through the linked list to listen to the first succeeds */
  for (walker = server_info; walker != NULL; walker = walker->ai_next) {
    /* a successful workflow would be socket() -> setsockopt() -> bind() ->
     * listen() */
    if ((server_fd = socket(walker->ai_family, walker->ai_socktype,
                            walker->ai_protocol)) == -1) {
      if (verbose) {
        perror("server: socket()");
      }
      continue;
    }

    // allow re-usage of the same port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) ==
        -1) {
      if (verbose) {
        perror("server: setsockopt()");
      }
      close(server_fd);
      continue;
    }

    if (bind(server_fd, walker->ai_addr, walker->ai_addrlen) == -1) {
      if (verbose) {
        perror("server: bind()");
      }
      close(server_fd);
      continue;
    }

    if (listen(server_fd, backlog) == -1) {
      if (verbose) {
        perror("server: listen()");
      }
      close(server_fd);
      continue;
    }

    break;  // succeed in building a server socket
  }

  if (walker == NULL) {
    if (verbose) {
      perror("server: fail to bind-listen to any");
    }
    freeaddrinfo(server_info);  // done with this linked list
    return -1;
  }

  if (verbose) {
    char server[INET6_ADDRSTRLEN];
    inet_ntop(walker->ai_family, get_addr_in(walker->ai_addr), server,
              sizeof server);
    fprintf(stdout, "server: bind-listen with address [%s] on port [%s]\n",
            server, port);
  }

  freeaddrinfo(server_info);  // done with this linked list

  return server_fd;
}

/**
 * @brief Ensure to read in as many as len bytes of data into user provided
 * buffer. It will sit-wait until read in all the required many bytes.
 * @param fd the file descriptor to read from, typically socket
 * @param buf the buffer to place the data read from fd
 * @param len how many bytes to read from the fd
 * @attention user must ensure the buffer has at least len many space available
 * @return ssize_t how many bytes read, or -1 if any error happens
 */
ssize_t robust_read(int fd, void *buf, const size_t len) {
  static int flag = 0;  // most of cases
  ssize_t read;
  ssize_t total_read = len;
  ssize_t curr_read = 0;
  char *buf_next = (char *)buf;
  while (curr_read != total_read) {
    if ((read = recv(fd, buf_next, len, flag)) < 0) {
      return -1;
    } else if (read == 0) {
      // EOF
      return curr_read;
    }
    curr_read += read;
    buf_next += read;
  }
  return curr_read;
}

/**
 * @brief Ensure to write out as many as len bytes of data from the user
 * provided buffer It will sit-wait until write out all the required many bytes
 * @param fd the file descriptor, typically socket
 * @param buf the buffer which contains data to be written into fd
 * @param len how many bytes to write into the fd
 * @attention user must ensure the buffer has at least len many space available
 * @return ssize_t how many bytes written, or -1 if any error happens
 */
ssize_t robust_write(int fd, const void *buf, const size_t len) {
  static int flag = 0;  // most of cases
  ssize_t write;
  ssize_t total_write = len;
  ssize_t curr_write = 0;
  const char *buf_next = (const char *)buf;
  while (curr_write != total_write) {
    if ((write = send(fd, buf_next, len - curr_write, flag)) <= 0) {
      if (errno != EINTR) {
        return -1;
      }
      write = 0;  // Interrupted by sig handler return, call write() again
    }
    buf_next += write;
    curr_write += write;
  }
  return curr_write;
}

/**
   @brief poll() functionality support.
   notice it always use malloc() and free() regardless of compiling on C or C++.
   therefore, user must adhere to call init_poll_array() and
   release_poll_array(), instead of attempting to release the allocated space
   themselves
*/
typedef struct poll_array {
  struct pollfd *pfds;  // points to the array of struct pollfd for poll
  nfds_t count;         // how many are there in the array pfds
  nfds_t capacity;      // the underlying allocated space for pfds
} poll_array_t;

/**
 * @brief Initialize an poll array with default capacity.
 * @return poll_array_t* pointer to the new allocated poll array struct
 */
poll_array_t *init_poll_array() {
  poll_array_t *new_poll_array = (poll_array_t *)malloc(sizeof(poll_array_t));
  new_poll_array->pfds =
      (struct pollfd *)malloc(INIT_POLL_ARRAY_CAPACITY * sizeof(struct pollfd));
  new_poll_array->capacity = INIT_POLL_ARRAY_CAPACITY;
  new_poll_array->count = 0;
  return new_poll_array;
}

/**
 * @brief Add a new socket descriptor under poll monitoring.
 * User should ensure that no duplicate fd is added into the array.
 * This functionality doesn't check for duplicate inserts.
 * @param new_fd new socket descriptor to be added
 * @param array pointer to the allocated poll array struct
 * @param flag the bit flag for the descriptor to be monitored upon
 */
void add_to_poll_array(int new_fd, poll_array_t *array, short flag) {
  static int expand_factor = 2;
  if (array->count == array->capacity) {
    // allocated space expansion needed
    array->capacity *= expand_factor;
    array->pfds = (struct pollfd *)realloc(
        array->pfds, array->capacity * sizeof(struct pollfd));
  }
  memset(&array->pfds[array->count], 0, sizeof(struct pollfd));
  array->pfds[array->count].fd = new_fd;
  array->pfds[array->count].events = flag;

  array->count += 1;
}

/**
 * @brief Remove an indexed socket descriptor from the poll array
 * User should ensure the index lies in between [0, array->count)
 * If the index out of bound, the program will exit with code 1
 * @param remove_idx the to-be-removed index from the poll array
 * @param array pointer to the allocated poll array struct
 */
void remove_from_poll_array(int remove_idx, poll_array_t *array) {
  static int shrink_criteria = 4;
  static int shrink_factor = 2;
  if (remove_idx < 0 || remove_idx >= array->count) {
    perror("remove_from_poll_array(): index out of bound");
    exit(1);
  }
  // close the socket descriptor
  close(array->pfds[remove_idx].fd);
  // swap the last entry into the removed index
  array->pfds[remove_idx] = array->pfds[array->count - 1];
  array->count -= 1;
  if (array->count < array->capacity / shrink_criteria) {
    // allocated space shrinkage needed
    array->capacity /= shrink_factor;
    array->pfds = (struct pollfd *)realloc(
        array->pfds, array->capacity * sizeof(struct pollfd));
  }
}

/**
 * @brief Release the dynamical allocated space for poll array
 * @param array pointer to the allocated poll array
 */
void release_poll_array(poll_array_t *array) {
  for (int i = array->count - 1; i >= 0; i--) {
    remove_from_poll_array(i, array);
  }
  free(array->pfds);
  free(array);
}

#endif /* SOCKET_UTILS_H */