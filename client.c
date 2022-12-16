#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "ufs.h"

#define BUFFER_SIZE (1000)

// client code
int main(int argc, char *argv[]) {
    printf("size of inode: %ld\n", sizeof(inode_t));
    // initialize port connection:
    MFS_Init("localhost", 52364);

    // { // TEST 1: Test stat
    //     int inum2 = 0;
    //     MFS_Stat_t stat;
    //     printf("before mfs\n");
    //     MFS_Stat(inum2, &stat);
    // }
    
   

    { // TEST 3: Test Creat
        int pinum = 0;
        printf("before creat\n");
        MFS_Creat(pinum, 1, "sexy file yum");
    }

    //  { // TEST 2: Test lookup
    //     int pinum = 0;
    //     printf("before lookup \n");
    //     MFS_Lookup(pinum, "idk");
    // }

    { // TEST 4: Test Write
    int inum = 1; // file inum
    char *buffer = "writing to the file\n";
    int offset = 69;
    int nbytes = strlen(buffer) + 1;
    printf("nbytes is: %d\n", nbytes);

    MFS_Write(inum, buffer, offset, nbytes);

    sleep(1);
    

     // TEST 5: Test Read
    int inum2 = 1; // file inum
    char buffer2[MFS_BLOCK_SIZE];
    int offset2 = 69;
    int nbytes2 = strlen(buffer) + 1;
    printf("nbytes2 is: %d\n", nbytes2);
    MFS_Read(inum2, buffer2, offset2, nbytes2);
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


