#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (1000)


int serverfd;
messagestruct *msg;


void intHandler(int dummy) {
    UDP_Close(serverfd);
    exit(130);
}

// server MFS_Stat
char* Ser_MFS_Stat(){
//    return (char*)(&(msg->statstruct));

    MFS_Stat_t *statstruct = &(msg->statstruct);
    statstruct->type = 0;
    statstruct->size = 100;
    
    return (char*)statstruct;
 }

// server code
int main(int argc, char *argv[]) {

    //OFFICIAL CODE, DO NOT DELETE:

    // assert(argc == 3);
    // int port_num = atoi(argv[1]);
    // const shar* img_path = argv[2];

    // sd = UDP_Open(port_num);
    // if(sd <= 0){
    //     //throw error
    //     perror("Failed to bind to UDP socket\n");
    //     return -1;
    // } else{
    //     printf("Listening for connections on port %i\n", port_num);
    // }

    // int file_fd = open(img_path, O_RDWR | O_APPEND);
    // struct stat finfo;

    // if(fstat(file_fd, &finfo) != 0) {
    //     perror("Fstat failed\n");
    // }

    // void *fs_img = mmap(NULL, finfo.st_size, MAP_SHARED, PROT_READ | PROT_WRITE, file_fd, 0);

    // super_t *superblock = superblock->inode_bitmap_len * sizeof(unsigned int)* 8;

    // END KAI'S CODE


    serverfd = UDP_Open(10000);
    assert(serverfd > -1);

    while (1) {
      struct sockaddr_in addr;
      char message[BUFFER_SIZE];
      printf("server:: waiting...\n");
      int rc = UDP_Read(serverfd, &addr, message, BUFFER_SIZE);

      //working till here
      printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
      msg = (messagestruct*)message;
      char *tempreply;

      if(msg->type == STAT) {
        printf(" message inum: %d\n", msg->inum);

        tempreply = Ser_MFS_Stat();        
        
      }

      // working after here
      if (rc > 0) {
        char reply[BUFFER_SIZE];
        sprintf(reply, "ok");
        rc = UDP_Write(serverfd, &addr, tempreply, sizeof(MFS_Stat_t));
        printf("server:: reply\n");
      } 
    }

    signal(SIGINT, intHandler);


    return 0; 
}

// (char *)&stat stuct;

//  user input:
//  int MFS_Stat(int inum, MFS_Stat_t *m);

// typedef struct __MFS_Stat_t {
//     int type;   // MFS_DIRECTORY or MFS_REGULAR
//     int size;   // bytes
//     // note: no permissions, access times, etc.
// } MFS_Stat_t;