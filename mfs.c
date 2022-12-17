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
//         // //("Data is available now.\n");
//         /* FD_ISSET(0, &rfds) will be true. */
//         rc = UDP_Read(sd, &addr2, (char *)&buf, sizeof(MFS_Msg_t));
//         //("CLIENT:: Type: %d Inum: %d Block: %d Message: %s\n",buf.type,buf.inum,buf.block,(char *)buf.buffer);
//         return rc;
//       }
//       else {
//         ////("No data within five seconds.\n");
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
  // //("in mfs lookup\n");

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


  // //("read %d bytes\n", n);
  int ret = (int)*response;
  // //("return of lookup: %d\n", ret);
  // for (int i = 0; i < 8; i++) {
  //   for (int j = 0; j < 8; j++) {
  //     //("%x ", response[i*8 + j]);
  //   }
  //   //("\n");
  // }
  //TODO: Fix stack smashing error and add string to seaarch for in lookup
  // fflush(stdout);
  if(ret == -2){
    ret = -1;
  }
  return ret;
}

// Done
int MFS_Stat(int inum, MFS_Stat_t *m) {
  // //("in mfs stat\n");

  msg.type = STAT;
  msg.statstruct = *m;
  msg.inum = inum;


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  MFS_Stat_t *statreturn = malloc(sizeof(MFS_Stat_t));
  // m = malloc(sizeof(MFS_Stat_t));

  n = UDP_Read(clientfd, &addrRcv, statreturn, sizeof(MFS_Stat_t));
  if (n < 0) {
    perror("read");
    return -1;
  }

  // m = (MFS_Stat_t*)statreturn;
  // //("size of m: %d\n", m->size);
  // //("type of data: %d\n", m->type);
  fflush(stdout);
  m->size = statreturn->size;
  m->type = statreturn->type;

  // return 0 on success
  return 0;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes) {
  // //("in mfs write\n");

  msg.type = WRITE;
  msg.inum = inum;
  msg.offset = offset;
  msg.nbytes = nbytes;
  // //("seg fault?\n");
  memcpy(msg.buffer, buffer, nbytes);

  // //("why though?\n");
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

  int a = (int)*response;
  //("mfs write response %d\n", a);

  return a;
}


int MFS_Read(int inum, char *buffer, int offset, int nbytes) {

  msg.type = READ;
  msg.inum = inum;
  msg.offset = offset;
  msg.nbytes = nbytes;
  // sprintf(msg.buffer, buffer);


  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  char *response = malloc(msg.nbytes);

  // n = UDP_Read(clientfd, &addrRcv, response, sizeof(int*));
  n = UDP_Read(clientfd, &addrRcv, response, msg.nbytes);
  // //("%s\n", response);

  if (n < 0) {
    perror("read");
    return -1;
  }


  if((int)*response == -1){
    return -1;
  }

  memcpy(buffer, response, msg.nbytes);

  if(strcmp(response, "-1")==0){
    return -1;
  }

  // //("read %d bytes\n", n);
  // memcpy(buffer, response, msg.nbytes);
  // //("response of creat: %s\n", (int)*response);
  
  // return (int)*response;
  return 0;
}

int MFS_Creat(int pinum, int type, char *name) {
  // //("in mfs creat\n");

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

  // //("read %d bytes\n", n);
  int a = (int)*response;
  // //("response of creat: %d\n", a);
  return a;

}

int MFS_Unlink(int pinum, char *name) {
  // //("in mfs unlink\n");

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

    int a = (int)*response;
  // //("response of unlink: %d\n", a);
  return a;

}

int MFS_Shutdown() {
  // //("in mfs shutdown\n");

  msg.type = SHUTDOWN;

  int n = UDP_Write(clientfd, &addrSnd, (char *)&msg, sizeof(messagestruct));
  if (n < 0) {
    perror("write");
    return -1;
  }

  UDP_Close(clientfd);
}