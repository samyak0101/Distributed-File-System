#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include <unistd.h>
#include "udp.h"
#include "mfs.h"
#include <time.h>
#include <stdlib.h>
#define MFS_NAME_LEN 32
#define BUFFER_SIZE (1000)
// #define MFS_BLOCK_SIZE 4096

int clientport;
static int serverport;
static int serverfd;
static int clientfd;
static struct sockaddr_in addrSnd, addrRcv; 
messagestruct msg;
int MIN_PORT = 20000;
int MAX_PORT = 40000;





// int tryMessage(struct _message message) 
// {    
//   int retval, rc;
//   // do {  
//     // tv.tv_sec = 30;
//     // tv.tv_usec = 0;
//     // FD_ZERO(&rsds);
//     // FD_SET(sd,&rsds);
//     rc = UDP_Write(clientfd, &addr, (char *)&message, sizeof(_message));
//     if (rc > 0) {
//       retval = select(sd+1, &rsds, NULL, NULL, &tv);
//       if (retval == -1)
//         perror("select()");
//       else if (retval) {
//         // printf("Data is available now.\n");
//         /* FD_ISSET(0, &rfds) will be true. */
//         rc = UDP_Read(sd, &addr2, (char *)&buf, sizeof(MFS_Msg_t));
//         printf("CLIENT:: Type: %d Inum: %d Block: %d Message: %s\n",buf.type,buf.inum,buf.block,(char *)buf.buffer);
//         return rc;
//       }
//       else {
//         //printf("No data within five seconds.\n");
//       }
//     }
//   // } while (!retval);
//   return -1;
// }


int MFS_Init(char *hostname, int port) {
  serverport = port;

  // opens client port and sets socket fd to be clientfd

  srand(time(0));
  clientport = (rand() % (MAX_PORT - MIN_PORT) + MIN_PORT);
  clientfd = UDP_Open(clientport);
  if(clientfd < 0){
    // TODO: throw error
  }

  // sets the destination port to send messages to
  serverfd = UDP_FillSockAddr(&addrSnd, hostname, serverport);
  if(serverfd < 0){
    // TODO: throw error
  }

    return 0;
}

int MFS_Lookup(int pinum, char name[28]) {
  printf("in mfs lookup\n");

  msg.type = LOOKUP;
  msg.pinum = pinum;
  sprintf(msg.name, name);


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char response[4096];

  n = UDP_Read(clientfd, &addrRcv, response, sizeof(int *));
  if (n < 0) {
    perror("read");
    return -1;
  }

  printf("read %d bytes\n", n);
  printf("inum of lookup file: %d\n", (int)*response);
  // for (int i = 0; i < 8; i++) {
  //   for (int j = 0; j < 8; j++) {
  //     printf("%x ", response[i*8 + j]);
  //   }
  //   printf("\n");
  // }
  //TODO: Fix stack smashing error and add string to seaarch for in lookup
  // fflush(stdout);

  return 0;
}

// Done
int MFS_Stat(int inum, MFS_Stat_t *m) {
  printf("in mfs stat\n");

  msg.type = STAT;
  msg.statstruct = *m;
  msg.inum = inum;


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char statreturn[sizeof(MFS_Stat_t)];
  n = UDP_Read(clientfd, &addrRcv, statreturn, sizeof(MFS_Stat_t));
  if (n < 0) {
    perror("read");
    return -1;
  }

  m = (MFS_Stat_t*)statreturn;
  printf("size of m: %d\n", m->size);
  printf("type of data: %d\n", m->type);
  fflush(stdout);

  // return 0 on success
  return 0;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes) {
  printf("in mfs write\n");

  msg.type = WRITE;
  msg.inum = inum;
  msg.offset = offset;
  msg.nbytes = nbytes;
  sprintf(msg.buffer, buffer);


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char response[4096];

  n = UDP_Read(clientfd, &addrRcv, response, sizeof(int *));
  if (n < 0) {
    perror("read");
    return -1;
  }

  printf("read %d bytes\n", n);
  printf("response of write: %d\n", (int)*response);


return 0;
}


int MFS_Read(int inum, char *buffer, int offset, int nbytes) {
  printf("in mfs read\n");

  msg.type = READ;
  msg.inum = inum;
  msg.offset = offset;
  msg.nbytes = nbytes;
  sprintf(msg.buffer, buffer);


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char response[4096];

  n = UDP_Read(clientfd, &addrRcv, response, sizeof(int *));
  if (n < 0) {
    perror("read");
    return -1;
  }

  printf("read %d bytes\n", n);
  printf("response of read: %d\n", (int)*response);

return (int)*response;
}

int MFS_Creat(int pinum, int type, char *name) {
  printf("in mfs creat\n");

  msg.type = CREAT;
  msg.pinum = pinum;
  msg.ttype = type;
  sprintf(msg.name, name);

  if(strlen(name)>27){
    return -1;
  }

  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char response[4096];

  n = UDP_Read(clientfd, &addrRcv, response, sizeof(int *));
  if (n < 0) {
    perror("read");
    return -1;
  }

  printf("read %d bytes\n", n);
  printf("response of creat: %d\n", (int)*response);
  return 0;

}

int MFS_Unlink(int pinum, char *name) {
  printf("in mfs unlink\n");

  msg.type = UNLINK;
  msg.pinum = pinum;
  sprintf(msg.name, name);


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char response[4096];

  n = UDP_Read(clientfd, &addrRcv, response, sizeof(int *));
  if (n < 0) {
    perror("read");
    return -1;
  }

  printf("read %d bytes\n", n);
  printf("response of unlink: %d\n", (int)*response);
  return 0;

}

int MFS_Shutdown() {
  printf("in mfs shutdown\n");

  msg.type = SHUTDOWN;

  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  UDP_Close(clientfd);
}