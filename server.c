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
inode_t *inodes;
int max_inodes;
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
// Question: shouldn't name in mfs.c MFS_Lookup be a 28 bit char array?
 int Ser_MFS_Lookup(void* fs_img){

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
    unsigned int location = inode->direct[0];
    location = location * UFS_BLOCK_SIZE;
    inode_offset = fs_img + location;
    dir_ent_t *dirent = (dir_ent_t *)inode_offset; // was MFS_DirEnt_t before
    char *name = dirent->name;
    printf("printing dir name\n%s\n", name);

    // return inum
    return dirent->inum;
 }

// server MFS Creat
int Ser_MFS_Creat(void* fs_img){
    printf("Message Type: %d\n", msg->type);
    // check if the inode of the parent directory is valid
    unsigned int *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    unsigned int valid = get_bit(addr, msg->pinum);
    if(valid != 1){
        return -1;
    }
    // else, go to inode of parent directory and 
    unsigned int *parent = fs_img + (superblock->inode_region_addr + msg->pinum/32) * MFS_BLOCK_SIZE;
    parent = parent + (msg->pinum % 32);
    inode_t *parentinode = (inode_t*)parent;
    unsigned int checklocation;
    // see if parent if parent directory is full or not, return -1 if full
    for(int i = 0; i < DIRECT_PTRS; i++){
      checklocation = parentinode->direct[i];
      dir_ent_t *check = (dir_ent_t*)(&checklocation);
      // if an unused directory entry exists, there is space to create a new file/directory
      if(check->inum == -1){
        break;
      }
      // if entire direct array is being used, no room to create new file/directory
      if(i == DIRECT_PTRS - 1){
        return -1;
      }
    }
    // check if file or directory you're attempting to create already exists
    for(int i = 0; i < DIRECT_PTRS; i++){
      checklocation = parentinode->direct[i];
      // unsigned int *check_offset = fs_img + (superblock->inode_region_addr + index/32)*MFS_BLOCK_SIZE + (index % 32);
      dir_ent_t *check = (dir_ent_t*)(&checklocation);
      // if file/directory already exists, return 0
      if(strcmp(msg->name, check->name) == 0)
        return 0;
    }
    // if file/directory does not exist, allocate inode for it
    unsigned int *tempint = fs_img + superblock->inode_bitmap_addr * MFS_BLOCK_SIZE;
    // inode_t *allocinode = (inode_t*)tempint;
    // dir_ent_t *allocdirent = (dir_ent_t*)tempint;
    int *ibitmapalloc = (int*)tempint;
    for(int i = 0; i < DIRECT_PTRS; i++){
      // find unallocated bit in i-bitmap
      int bit = (*ibitmapalloc >> (31 - i)) & 0x1;
      if(bit == 0){
        // allocate bit in i-bitmap
        *ibitmapalloc |= (0x1 << (31 - i));
        // allocate inode from iblocks
        tempint = fs_img + (superblock->inode_region_addr * MFS_BLOCK_SIZE) + (i * sizeof(inode_t));
        inode_t *inodealloc = (inode_t*)tempint;
        inodealloc->type = msg->ttype;
        inodealloc->size = 0;

        int direntnum;
        // create new directory entry in parent directory
        // j starts at 2 instead of 0 because directory entries 0 and 1 are for . and ..
        for(int j = 2; j < DIRECT_PTRS; j++){
          checklocation = parentinode->direct[j];
          dir_ent_t *direntalloc = (dir_ent_t*)(&checklocation);
          // if current directory entry is unused, allocate space for new file/directory
          if(direntalloc->inum == -1){
            // i comes from outer for loop (index of allocated inode)
            direntalloc->inum = i;
            // name comes from message passed from client
            // name being too long should be checked in mfs.c
            strcpy(direntalloc->name, msg->name);
            // direntnum used when allocating space in data region below
            direntnum = j;
          }
        }

        // TODO: UNFUCK THIS CODE BELOW, YOU GOT THE ALLOCATION OF DIRECTORY ENTRIES WRONG

        // if creating a directory and not a file, need to find location for it in data region
        // first find unallocated bit in d-bitmap
        if(inodealloc->type == MFS_DIRECTORY){
          tempint = fs_img + superblock->data_bitmap_addr * MFS_BLOCK_SIZE;
          int *dbitmapalloc = (int*)tempint;
          for(int j = 0; j < DIRECT_PTRS; j++){
            int bit = (*dbitmapalloc >> (31 - j)) & 0x1;
            if(bit == 0){
              // allocate bit in d-bitmap
              *dbitmapalloc |= (0x1 << (31 - j));
              // point directory entry address to corresponding part of data region
              tempint = fs_img + ((superblock->data_bitmap_addr + j) * MFS_BLOCK_SIZE);
              parentinode->direct[direntnum] = tempint;
              
              // in newly allocated inode, since we are creating a directory,
              // set first two directory entries to . and .. respectively
              // tempint = fs_img + (superblock->inode_region_addr * MFS_BLOCK_SIZE) + (i * sizeof(inode_t));
              unsigned int tempdirentint = fs_img + ((superblock->data_region_addr + j) * MFS_BLOCK_SIZE);
              inodealloc->direct[0] = tempdirentint;
              inodealloc->direct[1] = (tempdirentint + sizeof(dir_ent_t));
              tempint = inodealloc->direct[0];
              dir_ent_t *direntalloc = (dir_ent_t*)(&tempint);
              direntalloc->inum = msg->pinum;
              strcpy(direntalloc->name, ".");
              direntalloc = direntalloc + sizeof(dir_ent_t);
              direntalloc->inum = i;
              strcpy(direntalloc->name, "..");
              break;
            }
            // if no room in data region, directory is still created but 
            if(j == DIRECT_PTRS - 1){
              // TODO: Might have to move this section so that an inode is never allocated for a directory
              // when there is no room for the directory's entries; gonna wait and find out if this is needed
              return -1;
            }
          }
        }
        // only one inode should be updated allocated at a time for file/directory creation
        break;
      }
    }
    return 0;
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
    max_inodes = superblock->inode_bitmap_len * sizeof(unsigned int)* 8;

    inodes = fs_img + superblock->inode_region_addr * UFS_BLOCK_SIZE;

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