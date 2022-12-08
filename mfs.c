#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "mfs.h"

#define MFS_NAME_LEN 32
#define MFS_BLOCK_SIZE 4096

typedef struct MFS_Stat_t {
  int type;  // MFS_DIRECTORY or MFS_REGULAR
  int size;  // bytes
  int blocks;  // number of blocks allocated to file
  // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct MFS_DirEnt_t {
  char name[MFS_NAME_LEN];  // filename
  int  inum;                // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

int MFS_Init(char *hostname, int port) {
  // Create a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return -1;
  }

  // Connect to the server
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  inet_aton(hostname, &server.sin_addr);
  if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0) {
    perror("connect");
    return -1;
  }

  return sockfd;
}

int MFS_Lookup(int pinum, char *name) {
  int inum;
  int n = write(pinum, &inum, sizeof(inum));
  if (n < 0) {
    perror("write");
    return -1;
  }

  n = read(pinum, &inum, sizeof(inum));
  if (n < 0) {
    perror("read");
    return -1;
  }

  return inum;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
  int n = write(inum, m, sizeof(MFS_Stat_t));
  if (n < 0) {
    perror("write");
    return -1;
  }

  n = read(inum, m, sizeof(MFS_Stat_t));
  if (n < 0) {
    perror("read");
    return -1;
  }

  return 0;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes) {
  int n = write(inum, buffer, nbytes);
  if (n < 0) {
    perror("write");
    return -1;
  }

  return 0;
}

int MFS_Read(int inum, char *buffer, int offset, int nbytes) {
  int n = write(inum, buffer, nbytes);
  if (n < 0) {
    perror("write");
    return -1;
  }

  n = read(inum, buffer, nbytes);
  if (n < 0) {
    perror("read");
    return -1;
  }

  return 0;
}

int MFS_Creat(int pinum, int type, char *name) {
  int n = write(pinum, &type, sizeof(type));
  if (n < 0) {
    perror("write");
    return -1;
  }

  n = write(pinum, name, MFS_NAME_LEN);
  if (n < 0) {
    perror("write");
    return -1;
  }

  n = read(pinum, &type, sizeof(type));
  if (n < 0) {
    perror("read");
    return -1;
  }

  return 0;
}

int MFS_Unlink(int pinum, char *name) {
  int n = write(pinum, name, MFS_NAME_LEN);
  if (n < 0) {
    perror("write");
    return -1;
  }

  n = read(pinum, name, MFS_NAME_LEN);
  if (n < 0) {
    perror("read");
    return -1;
  }

  return 0;
}

int MFS_Shutdown() {
  exit(0);
}