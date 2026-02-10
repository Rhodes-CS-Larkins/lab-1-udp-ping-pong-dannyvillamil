/*
 * ping.c - UDP ping/pong client code
 *          author: <Daniel Villamil>
 */
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#define PORTNO "1266"

int main(int argc, char **argv) {
  int ch, errors = 0;
  int nping = 1;                        // default packet count
  char *ponghost = strdup("localhost"); // default host
  char *pongport = strdup(PORTNO);      // default port
  int arraysize = 100;                  // default packet size

  int s;

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr,
              "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  // create array of packets (=200)
  unsigned char array[arraysize];
  memset(array, 200, arraysize);

  // create server address
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP

  if (getaddrinfo(ponghost, pongport, &hints, &res) != 0) {
    perror("failed getaddrinfo");
    exit(1);
  }

  // create socket
  s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (s < 0) {
    perror("failed socket");
    exit(1);
  }

  double total_time = 0.0;

  // Ping loop
  for (int i = 0; i < nping; i++) {

    double start = get_wctime();

    // send packet to server
    if (sendto(s, array, arraysize, 0, res->ai_addr, res->ai_addrlen) < 0) {
      perror("failed to send");
      exit(1);
    }

    // receive pong
    if (recvfrom(s, array, arraysize, 0, NULL, NULL) < 0) {
      perror("failed to receive");
      exit(1);
    }

    double end = get_wctime();

    double rtt = (end - start) * 1000.0;

    // validate reply
    int valid = 1;
    for (int j = 0; j < arraysize; j++) {
      if (array[j] != 201) {
        valid = 0;
        break;
      }
    }

    if (valid) {
      printf("ping[%d] : round-trip time: %.3f ms\n", i, rtt);
      total_time += rtt;
    } else {
      errors++;
    }

    // new ping (reset)
    memset(array, 200, arraysize);
  }

  // print summary
  if (errors == 0) {
    printf("no errors detected\n");
  } else {
    printf("%d errors detected\n", errors);
  }
  printf("time to send %d packets of %d bytes %.3f ms (%.3f avg per packet)\n",
         nping, arraysize, total_time, total_time / nping);

  // cleanup
  freeaddrinfo(res);
  close(s);
  return 0;
}
