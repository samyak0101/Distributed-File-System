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

    MFS_Creat(0, 0, "testdir");
    int inum = MFS_Lookup(0, "testdir");
    printf("Inum: %d\n", inum);

    int res = MFS_Lookup(inum, ".");
    if (res != inum) {
        printf("Error 0 %d %d\n", res, inum);
        MFS_Shutdown();
        exit(1);
    }

    /*if (MFS_Lookup(inum, "..") != 0) {
        printf("Error 1 %d\n", inum);
        exit(1);
    }*/

    MFS_Shutdown();


    exit(0);
}