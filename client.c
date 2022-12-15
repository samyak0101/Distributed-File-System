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
    



//    { // TEST 2: Test lookup
//         int pinum = 0;
//         printf("before lookup \n");
//         MFS_Lookup(pinum, "test 5");
//     }





    // { // TEST 3: Test Creat
    //     int pinum = 0;
    //     printf("before creat\n");
    //     MFS_Creat(pinum, 1, "dir 1 write");
    // }



    { // TEST 4: Test Write
    int inum = 1;// file inum
    char *buffer = "writing to the file\n";
    int offset = 0;
    int nbytes = sizeof(buffer);
    printf("nbytes is: %d\n", nbytes);

    // MFS_Write(inum, buffer, offest, nbytes);

    return 0;
    }





}


