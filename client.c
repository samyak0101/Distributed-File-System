#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "ufs.h"
#include <unistd.h>

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
        MFS_Creat(pinum, 0, "dir 1");
    }

     sleep(5);


   { // TEST 2: Test lookup
        int pinum = 0;
        printf("before lookup \n");
        int a = MFS_Lookup(pinum, "dir 1");
        printf("lookup response: %d\n", a);
    }

    sleep(5);

    { // TEST 3: Test Creat
        int pinum = 1;
        printf("before creat\n");
        int a = MFS_Creat(pinum, 0, "dir 1a");
        printf("creat response: %d\n", a);

    }

    sleep(5);

    { // TEST 3: Test Creat
        int pinum = 1;
        printf("before creat\n");
        MFS_Creat(pinum, 0, "dir 1b");
    }

    //  sleep(5);

    { // TEST 1: Test stat
        int inum2 = 3;
        MFS_Stat_t *stat;
        // stat = malloc(sizeof(MFS_Stat_t));
        printf("before mfs\n");
        MFS_Stat(inum2, stat);
        printf("type: %d\n and size: %d\n", stat->size, stat->type);
    }
    


    // { // TEST 4: Test Write
    // int inum = 1;// file inum
    // char *buffer = "writing to the file\n";
    // int offset = 0;
    // int nbytes = sizeof(buffer);
    // printf("nbytes is: %d\n", nbytes);

    // MFS_Write(inum, buffer, offest, nbytes);

    // }


    // sleep(10);

    
    // { // TEST 5: Test Unlink
    //     int pinum = 0;
    //     printf("before creat\n");
    //     int a = MFS_Unlink(pinum, "file to del");
    //     printf("unlink response: %d\n", a);
    // }


    return 0;

}


