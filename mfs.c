#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include <unistd.h>
#include "udp.h"
#include "mfs.h"

#define MFS_NAME_LEN 32
#define BUFFER_SIZE (1000)
// #define MFS_BLOCK_SIZE 4096

static int clientport = 39965;
static int serverport;
static int serverfd;
static int clientfd;
static struct sockaddr_in addrSnd, addrRcv; 
messagestruct msg;

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

int MFS_Lookup(int pinum, char *name) {
  printf("in mfs lookup\n");

  msg.type = LOOKUP;
  msg.pinum = pinum;
  msg.name = strdup(name);


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char smth[1];

  n = UDP_Read(clientfd, &addrRcv, smth, sizeof(int *));
  if (n < 0) {
    perror("read");
    return -1;
  }

  printf("inum of lookup file: %d\n", atoi(smth));
  fflush(stdout);

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
return 0;
}

// int rc;

// // added for testing
//     char message[BUFFER_SIZE];


//     sprintf(message, buffer);
//     // sending the message
//     printf("client:: send message [%s]\n", message);
//     rc = UDP_Write(clientfd, &addrSnd, message, BUFFER_SIZE);
//     if (rc < 0) {
// 	printf("client:: failed to send\n");
// 	exit(1);
//     }

//     // waiting for response
//     printf("client:: wait for reply...\n");
//     rc = UDP_Read(clientfd, &addrRcv, message, BUFFER_SIZE);
//     printf("client:: got reply [size:%d contents:(%s)\n", rc, message);


//   return 0;
// }

// int MFS_Read(int inum, char *buffer, int offset, int nbytes) {
//   int n = write(inum, buffer, nbytes);
//   if (n < 0) {
//     perror("write");
//     return -1;
//   }

//   n = read(inum, buffer, nbytes);
//   if (n < 0) {
//     perror("read");
//     return -1;
//   }

//   return 0;
// }

// int MFS_Creat(int pinum, int type, char *name) {
//   int n = write(pinum, &type, sizeof(type));
//   if (n < 0) {
//     perror("write");
//     return -1;
//   }

//   n = write(pinum, name, MFS_NAME_LEN);
//   if (n < 0) {
//     perror("write");
//     return -1;
//   }

//   n = read(pinum, &type, sizeof(type));
//   if (n < 0) {
//     perror("read");
//     return -1;
//   }

//   return 0;
// }

// int MFS_Unlink(int pinum, char *name) {
//   int n = write(pinum, name, MFS_NAME_LEN);
//   if (n < 0) {
//     perror("write");
//     return -1;
//   }

//   n = read(pinum, name, MFS_NAME_LEN);
//   if (n < 0) {
//     perror("read");
//     return -1;
//   }

//   return 0;
// }

// int MFS_Shutdown() {
//   exit(0);
// }