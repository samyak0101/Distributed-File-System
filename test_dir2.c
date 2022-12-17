#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "ufs.h"
#include <unistd.h>

#define BUFFER_SIZE (1000)

// client code
int main(int argc, char *argv[]) {
    MFS_Init("localhost", 52364);

    /*
    

      self.creat(ROOT, MFS_DIRECTORY, "testdir")
      inum = self.lookup(ROOT, "testdir")

      if self.lookup(inum, ".") != inum:
         raise Failure("'.' in directory should point to directory itself")

      if self.lookup(inum, "..") != ROOT:
         raise Failure("'..' in directory should point to parent")

      self.shutdown()
      
    */

    int write_one_block(int inum, char *write_block, int off, int nbytes){
      int ret = MFS_Write(inum, write_block, off, nbytes);
      // printf("now  :%s\n", ms); // should print same block as write!!!
      // printf("write return val: %d\n", ret); // should write a block into a file 
      sleep(0.1);
      return ret;
    }


    char *write_block = malloc(4096*sizeof(char));
    for (int i = 0; i<4096; i++){
      write_block[i] = 'a';
    }

    // printf("writing input: %ld\n", strlen(write_block));

    char *read_block = malloc(4096*sizeof(char));

    { // TEST 1: Test stat
      int inum2 = 0;
      MFS_Creat(0, 1, "testfile");
      int inum = MFS_Lookup(0, "testfile");


      for (int i = 0; i<31; i++){
        int ret = write_one_block(inum, write_block, UFS_BLOCK_SIZE*i, UFS_BLOCK_SIZE);
        printf("write return val: %d\n", ret); // should read
        printf("iteration number: %d\n", i);
        ret = MFS_Read(inum, read_block, UFS_BLOCK_SIZE*i, UFS_BLOCK_SIZE);
        printf("read return val: %d\n", ret);
      }

      // printf("now  :%s\n", ms); // should print same block as write!!!

      
      


    }

    // MFS_Creat(0, 0, "testdir");
    // int inum = MFS_Lookup(0, "testdir");
    // printf("Inum: %d\n", inum);

    // int res = MFS_Lookup(inum, ".");
    // if (res != inum) {
    //     printf("Error 0 %d %d\n", res, inum);
    // }

    // MFS_Creat(1, 0, "subdir 1");
    // inum = MFS_Lookup(1, "subdir 1");
    // printf("Inum: %d\n", inum);

    // MFS_Creat(1, 0, "subdir 2");
    // inum = MFS_Lookup(1, "subdir 2");
    // printf("Inum: %d\n", inum);

    // /*if (MFS_Lookup(inum, "..") != 0) {
    //     printf("Error 1 %d\n", inum);
    //     exit(1);
    // }*/



    // { // TEST 3: Test Creat
    //     int pinum = 0;
    //     printf("before creat\n");
    //     MFS_Creat(pinum, 0, "dir 1");
    // }

    //  sleep(2);


    // // TEST 2: Test look

    // // if (MFS_Lookup(1, ".") != 1) {
    // //     perror("Error\n");
    // //     exit(1);

    // // }
    //     int pinum = 0;
    //     printf("before lookup \n");
    //     int a = MFS_Lookup(pinum, "dir 1");
    //     printf("lookup response: %d\n", a);

    // sleep(2);

    // { // TEST 3: Test Creat
    //     int pinum = 1;
    //     printf("before creat\n");
    //     int a = MFS_Creat(pinum, 0, "dir-1a");
    //     printf("creat response: %d\n", a);

    // }

    //     printf("before lookup \n");
    //     a = MFS_Lookup(1, "dir-1a");
    //     printf("lookup response: %d\n", a);

    // sleep(2);

    // int rc = MFS_Unlink(0, "dir 1");
    // printf("Unlink RC: %d\n", rc );

   // { // TEST 3: Test Creat
   //      int pinum = 0;
   //      printf("before creat\n");
   //      int a = MFS_Creat(pinum, 1, "lick my sack");
   //      printf("creat response: %d\n", a);

   //  }


   //  for (int i = 0; i < 126; i++) {
   //      char name[6];
   //      int inum = 0;
   //      snprintf(name, 8, "test%d", i+1);
   //      int rc = MFS_Creat(inum, MFS_REGULAR_FILE, name); // inum = 1
   //      printf("in testdir.c, mfs create for %s returned %d\n", name, rc);
   //      printf("\n");
   //      rc = MFS_Lookup(inum, name);
   //      printf("in testdir.c, mfs lookup for %s returned %d\n", name, rc);
   //      printf("\n\n\n");
   //  }

   // MFS_Creat(0, 1, "test dir 32");

   //  { // TEST 4: Test Write
   //  int inum = 1;// file inum
   //  char *buffer = "writing to the file\n";
   //  int offset = 0;
   //  int nbytes = sizeof(buffer);
   //  printf("nbytes is: %d\n", nbytes);

   //  MFS_Write(inum, buffer, offset, nbytes);

   //  }

   //  { // TEST FORTNITE: Test Read
   //  int inum = 1; // file inum
   //  char buffer[4096];
   //  int offset = 0;
   //  int nbytes = sizeof(buffer);
   //  MFS_Read(inum, buffer, offset, nbytes);
   //  printf("data read: %s\n", buffer);

   //  }

   //  MFS_Shutdown();
   //  exit(0);


}