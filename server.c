#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "udp.h"
#include "mfs.h"
#include "ufs.h"

#define BUFFER_SIZE (1000)


int serverfd;
messagestruct *msg;
super_t *superblock;
int file_fd;

void intHandler(int dummy) {
    UDP_Close(serverfd);
    exit(130);
}

unsigned int get_bit(unsigned int *bitmap, int position) {
   int index = position / 32;
   int offset = 31 - (position % 32);
   return (bitmap[index] >> offset) & 0x1;
}

// server MFS_Stat
char* Ser_MFS_Stat(void* fs_img){
    printf("MESSAGE TYPE! type: %d\n", msg->type);
    MFS_Stat_t *statstruct = &(msg->statstruct);

    // check if the inode is valid in inode bitmap
    unsigned int *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    unsigned int valid = get_bit(addr, msg->inum);

    // return struct with -1's if inode is invalid
    printf("valid bit of root: %i\n", valid);
    if(valid != 1){
        statstruct->type = -1;
        statstruct->size = -1;
        return (char*)statstruct;
    }

    // else, go to inode and read in inode struct
    unsigned int *inode_offset = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE;
    inode_offset = inode_offset +  (msg->inum % 32);
    
    // read in inode struct but as an MFS stat struct
    statstruct = (MFS_Stat_t*)inode_offset;

    // return stat struct
    return (char*)statstruct;
 }

// server MFS Lookup
 int Ser_MFS_Lookup(void* fs_img){
    // TODO: fix method so it searches for string name in msg->name and return it's inum
    // Also do the error checking and retunrn appropriately when  the thing im looking for isn't there or something.
    printf("MESSAGE TYPE! type: %d\n", msg->type);

    // check if the inode is valid in inode bitmap
    unsigned int *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    unsigned int valid = get_bit(addr, msg->inum);

    // return struct with -1's if inode is invalid
    printf("valid bit of root: %i\n", valid);
    if(valid != 1){
        return -1;
    }

    // else, go to inode and read in inode struct
    unsigned int *inode_offset = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE;
    inode_offset = inode_offset +  (msg->inum % 32);
    // read in inode struct but as an MFS stat struct
    inode_t *inode = (inode_t*)inode_offset;
    int location = inode->direct[0];
    location = location * UFS_BLOCK_SIZE;
    inode_offset = fs_img + location;
    MFS_DirEnt_t *dirent = (MFS_DirEnt_t *)inode_offset;
    char *name = dirent->name;
    printf("printing dir name\n%s\n", name);

    // return inum
    return dirent->inum;
 }



// server code
int main(int argc, char *argv[]) {

    //OFFICIAL CODE, DO NOT DELETE:

    assert(argc == 3);
    int port_num = atoi(argv[1]);
    const char* img_path = argv[2];

    serverfd = UDP_Open(port_num);
    if(serverfd <= 0){
        //throw error
        perror("Failed to bind to UDP socket\n");
        return -1;
    } else{
        printf("Listening for connections on port %i\n", port_num);
    }

    file_fd = open(img_path, O_RDWR | O_APPEND);
    struct stat finfo;

    if(fstat(file_fd, &finfo) != 0) {
        perror("Fstat failed\n");
    }

    void *fs_img = mmap(NULL, finfo.st_size, MAP_SHARED, PROT_READ | PROT_WRITE, file_fd, 0);

    superblock = (super_t*)fs_img;
    int max_inodes = superblock->inode_bitmap_len * sizeof(unsigned int)* 8;

    inode_t *inodes = fs_img + superblock->inode_region_addr * UFS_BLOCK_SIZE;

    printf("inodes %d\n", inodes->type);
    
    printf("max inum: %i\n", max_inodes);

    // END KAI'S CODE


    // serverfd = UDP_Open(10000);
    // assert(serverfd > -1);

    while (1) {
      struct sockaddr_in addr;
      char message[BUFFER_SIZE];
      printf("server:: waiting...\n");
      int rc = UDP_Read(serverfd, &addr, message, BUFFER_SIZE);

      //working till here
      printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
      msg = (messagestruct*)message;
      printf("name in msg: %s\n", msg->name);
      char *tempreply;


        // TODO: Create switch statement for different message types
      if(msg->type == STAT) {
        printf(" message inum: %d\n", msg->inum);
        tempreply = Ser_MFS_Stat(fs_img);        
      }

      else if(msg->type == LOOKUP) {
        printf(" message pinum: %d\n", msg->pinum);
        int inum = Ser_MFS_Lookup(fs_img);
        printf("Lookup done, it's %d\n", inum);
        char a = (char)inum;
        tempreply = &a;
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