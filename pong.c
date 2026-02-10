/*
 * pong.c - UDP ping/pong server code
 *          author: <Daniel Villaml>
 */
#include <arpa/inet.h>
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
  int ch;
  int nping = 1;                   // default packet count
  char *pongport = strdup(PORTNO); // default port

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port]\n");
    }
  }

  // Create Server address
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, pongport, &hints, &res) != 0) {
    perror("failed getaddrinfo");
    exit(1);
  }

  // create socket
  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (s < 0) {
    perror("failed socket");
    exit(1);
  }

  // bind socket to port
  if (bind(s, res->ai_addr, res->ai_addrlen) != 0) {
    perror("failed bind");
    exit(1);
  }

  // receive and respond loop
  for (int i = 0; i < nping; i++) {
    char array[1024];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // receive packet
    ssize_t bytes = recvfrom(s, array, sizeof(array), 0,
                             (struct sockaddr *)&client_addr, &client_len);
    if (bytes < 0) {
      perror("failed to receive packet");
      exit(1);
    }

    // print client IP
    char ip[16];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, 16);
    printf("pong[%d]: received packet from %s\n", i, ip);

    // modify packet
    for (int j = 0; j < bytes; j++) {
      array[j] = (unsigned char)201;
    }

    // send back to client
    if (sendto(s, array, bytes, 0, (struct sockaddr *)&client_addr,
               client_len) < 0) {
      perror("failed to send");
      exit(1);
    }
  }

  // clean up
  freeaddrinfo(res);
  close(s);

  return 0;
}
