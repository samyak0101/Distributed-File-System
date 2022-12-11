#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "ufs.h"

#define BUFFER_SIZE (1000)

// client code
int main(int argc, char *argv[]) {
    inode_t inode;
    printf("size of inode: %ld\n", sizeof(inode_t));
    // initialize port connection:
    MFS_Init("localhost", 10000);

    // { // TEST 1: Test stat
    //     int inum2 = 0;
    //     MFS_Stat_t stat;
    //     printf("before mfs\n");
    //     MFS_Stat(inum2, &stat);
    // }
    
    { // TEST 2: Test lookup
        int pinum = 0;
        printf("before lookup \n");
        MFS_Lookup(pinum, "samyak jain lomalo");
    }

    return 0;
}









    // write message to server and receive a message back.
    // int inum = 1;
    // char *buffer = "hello world";
    // int offset = 0;
    // int nbytes = sizeof(buffer);
    // MFS_Write(inum,buffer,offset,nbytes);



    // int sd = UDP_Open(20000);
    // int rc = UDP_FillSockAddr(&addrSnd, "localhost", 10000);
    // printf("rc: %d\n", rc);

    // char message[BUFFER_SIZE];
    // sprintf(message, "hello world");

    // printf("client:: send message [%s]\n", message);
    // rc = UDP_Write(sd, &addrSnd, message, BUFFER_SIZE);
    // if (rc < 0) {
	// printf("client:: failed to send\n");
	// exit(1);
    // }

    // printf("client:: wait for reply...\n");
    // rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
    // printf("client:: got reply [size:%d contents:(%s)\n", rc, message);



