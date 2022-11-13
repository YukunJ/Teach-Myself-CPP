/*
** select.c -- a select() demo
*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN 0  // file descriptor for Standard Input

int main(void) {
  struct timeval tv;
  fd_set readfds;

  tv.tv_sec = 5;
  tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(STDIN, &readfds);

  // don't care about writefds and exceptfds
  select(STDIN + 1, &readfds, NULL, NULL, &tv);

  if (FD_ISSET(STDIN, &readfds)) {
    printf("A key was pressed!\n");
  } else {
    printf("Timed out.\n");
  }

  return 0;
}