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

void set_bit(unsigned int *bitmap, int position) {
  printf("position: %d\n", position);
   int index = position / 32;
   int offset = 31 - (position % 32);
   bitmap[index] |= (0x1 << offset);
}

//helper
int alloc_databitmap(void* fs_img){
  printf("inside alloc data bitmap \n");

  void *addr = fs_img + (superblock->data_bitmap_addr * MFS_BLOCK_SIZE);
  int valid;
  int position = -1;
  // TODO: change this loop to loop over entire data bitmap. Rn assuming only 1 block dbm.
  for (int i = 0; i < superblock -> num_data; i++){
    valid = get_bit((unsigned int *)addr, i);
    if(valid != 1){
      position = i;
      set_bit((unsigned int *)addr, i);
      break;
    }
  }
  printf("returned value databitmap alloc: %d\n", position);

  return position;  
}

//helper
int alloc_inodebitmap(void* fs_img){
  printf("inside alloc inode bit\n");

  void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
  int valid;
  int position = -1;
  // TODO: change this loop to loop over entire data bitmap. Rn assuming only 1 block dbm.
  for (int i = 0; i < superblock -> num_data; i++){
    valid = get_bit((unsigned int *)addr, i);
    if(valid != 1){
      position = i;
      set_bit((unsigned int *)addr, i);
      break;
    }
  }
  printf(" returned value inode bitmap alloc: %d\n", position);

  return position;  
}

// server MFS Stat
char* Ser_MFS_Stat(void* fs_img){
    printf("MESSAGE TYPE! type: %d\n", msg->type);
    MFS_Stat_t *statstruct = &(msg->statstruct);

    // check if the inode is valid in inode bitmap
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    unsigned int valid = get_bit(addr, msg->inum);

    // return struct with -1's if inode is invalid
    printf("valid bit of root: %i\n", valid);
    if(valid != 1){
        statstruct->type = -1;
        statstruct->size = -1;
        return (char*)statstruct;
    }

    // else, go to inode and read in inode struct
    void *inode_offset = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE;
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
    printf("Looking for this name in lookup: %s\n", msg->name);

    // check if the inode is valid in inode bitmap
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->inum);

    // return struct with -1's if inode is invalid
    printf("valid bit of root: %i\n", valid);
    if(valid != 1){
        return -1;
    }

    // else, go to inode and read in inode struct
    void *inode_offset = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE;
    inode_offset = inode_offset +  (msg->inum % 32);
    // read in inode struct but as an MFS stat struct
    inode_t *inode = (inode_t*)inode_offset;
    int found = -1;
    found -= 1; found += 1; found = -1;
    // loop through all directory pointers (30) and all dirents to find the file.
    for (int i = 0; i<DIRECT_PTRS; i++){
      // printf("entered for loop\n");
      if(inode->direct[i]<0 || inode->direct[i]>=30){
        break;
      }
      int location = inode->direct[i] * UFS_BLOCK_SIZE;
      // printf("block size: %ld\n", UFS_BLOCK_SIZE/sizeof(dir_ent_t));
      for (int j = 0; j < UFS_BLOCK_SIZE/sizeof(dir_ent_t); j++){
        // printf("entered second for loop\n");
        // location += sizeof(dir_ent_t)*j;
        void *dir_ent_offset = fs_img + location + sizeof(dir_ent_t)*j;
        dir_ent_t *directory_entry = (dir_ent_t *)dir_ent_offset;
        printf("name of dirent at this location: %s\n ", directory_entry->name);
        // check if the name is the same
        if(strcmp(msg->name, directory_entry->name)==0){
          found = 1;
          printf("Found this name in lookup: %s\n", directory_entry->name);
          return directory_entry->inum;
        }
      }
    }

    return -1;

}

// server MFS Creat
int Ser_MFS_Creat(void* fs_img){ 


    printf("Message Type: %d\n", msg->type);
    // check if the inode of the parent directory is valid
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->pinum);
    if(valid != 1){
      return -1; // pinum is invalid
    }
    printf("pinum is valid\n");


    //if pinum is valid, check if dir or file already exists
    int already_exists = Ser_MFS_Lookup(fs_img);
    if(already_exists != -1){
      printf("file or directory already exists\n");
      return 0; // file or directory with same name already exists, cannot overwrite
    } else {
      printf("file not found in lookup\n");
    }


    //go to inode of parent directory and find space for dirent
    void *parent_inode_addr = fs_img + (superblock->inode_region_addr + msg->pinum/32) * MFS_BLOCK_SIZE;
    parent_inode_addr = parent_inode_addr + sizeof(inode_t)*(msg->pinum % 32);
    // reading inode struct of parent.
    inode_t *parentinode = (inode_t*)parent_inode_addr;

    int direct_ptr_datablock;
    void *iterator;
    // see if parent if parent directory is full or not, return -1 if full
    dir_ent_t *check;

    // int found_dir_space = -1;
    int found = -1;

    // find space for dir ent of new file/directory and save pointer to address space check.
    printf("Looping through dirent to find free dir ent slot\n");
    for(int i = 0; i < DIRECT_PTRS; i++){
      printf("parent inode dierct i is %d\n", parentinode->direct[i]);
      if(parentinode->direct[i] == -1){
        printf("NOT SUPPOSED TO GO HERE! \n\n\n");
        // create and allocate here
        // TODO: assign data bitmap location and allocate data block
        int datablock_addr = alloc_databitmap(fs_img);
        if(datablock_addr == -1 && msg->type == 0){
          return -1;
          // cannot allocate since data bitmap is full
        }
        parentinode -> direct[i] = datablock_addr;
        iterator = fs_img + datablock_addr * UFS_BLOCK_SIZE;
        check = (dir_ent_t *)iterator;
        found = 1;
        break;
      }

      direct_ptr_datablock = parentinode->direct[i];
      iterator = fs_img + direct_ptr_datablock * UFS_BLOCK_SIZE;

      for (int j = 0; j < (int) UFS_BLOCK_SIZE / sizeof(dir_ent_t); j++){
        printf("checking for sub directories: \n");
        void *location = iterator + j*sizeof(dir_ent_t);
        check = (dir_ent_t *)location;
        // printf("name of this location: %s\n", check -> name);
        // if an unused directory entry exists, there is space to create a new file/directory
        // printf("Reached inum checking area\n");
        if(check->inum == -1){
          found = 1;
          break;
        }
      }
      
      if(found == 1){
        break;
        // found pointer, so can exit loop
      }

      if(i == DIRECT_PTRS - 1){
        return -1;
      }
      
      // if entire direct array is being used, no room to create new file/directory
    }

    if(found != 1){
      return -1;
    }
    //increase size of parent inode

    parentinode->size += sizeof(dir_ent_t);

    // file/directory does not exist, so allocate inode for it
    printf("allocating inode to type %d\n", msg->ttype);
    int new_inode = alloc_inodebitmap(fs_img);
    if(new_inode < 0){
      return -1; // could not allocate since inode bitmap is full
    }
    printf("Allocated inode : %d\n", new_inode);
    // creating inode in inode region
    void *tempint = fs_img + superblock->inode_region_addr * MFS_BLOCK_SIZE;
  
    tempint += sizeof(inode_t) * new_inode;

    // for (int i = 0; i<32; i++){
    //   void *k = fs_img + superblock->inode_region_addr * MFS_BLOCK_SIZE;
    //   k +=  
    //   inode_t *i = (inode_t *)
    // }

    inode_t *new_inode_struct = (inode_t *)tempint;
    new_inode_struct->type = msg->ttype;

    for (int p = 0; p<30; p++){
      new_inode_struct->direct[p] = -1;
    }

    if(msg->ttype == 0){
      printf("director: %d\n", msg->ttype);
    }
    else{
      printf("file: %d\n", msg->ttype);
    }
    if(msg->ttype==1){
      //almost done
      new_inode_struct -> size = 0;
      check->inum = new_inode;
      strcpy(check->name, msg->name);
      printf("Created new file. Inum is %d \n", new_inode);
      return 0; // done?
    }

    new_inode_struct -> size = 64;

    // if directory:
    printf("here\n");
    int newdir_dirent_int = alloc_databitmap(fs_img);
    if(newdir_dirent_int < 0){
      // set new direntry to null
      check = NULL;
      return -1;
    }

    new_inode_struct->direct[0] = superblock->data_region_addr + newdir_dirent_int;
    check->inum = new_inode;
    strcpy(check->name, msg->name);

    iterator = fs_img + new_inode_struct->direct[0] * UFS_BLOCK_SIZE;

      for (int j = 0; j < (int) UFS_BLOCK_SIZE / sizeof(dir_ent_t); j++){
        printf("checking for sub directories: \n");
        void *location = iterator + j*sizeof(dir_ent_t);
        check = (dir_ent_t *)location;
        check->inum = -1;
      }


    //write 2 dir ents to the new location
      void *inode_offset = fs_img + new_inode_struct->direct[0] * UFS_BLOCK_SIZE;
      dir_ent_t *newdir_dirent_one = (dir_ent_t *)inode_offset;
      
      // newdir_dirent_one->name = ".";
      printf("Created new directory. Inum is %d and about to write children now:\n", new_inode);
      strcpy(newdir_dirent_one->name, ".");
      newdir_dirent_one->inum = new_inode;
      newdir_dirent_one += 1;
      strcpy(newdir_dirent_one->name, "..");
      newdir_dirent_one->inum = msg->pinum;

    return 0;
}

// server MFS READ
char* Ser_MFS_Read(void* fs_img){
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

// server MFS WRITE
char* Ser_MFS_Write(void* fs_img){
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

// server MFS UNLINK
char* Ser_MFS_Unlink(void* fs_img){
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

// server MFS SHUTDOWN
char* Ser_MFS_Shutdown(void* fs_img){
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

    file_fd = open(img_path, O_RDWR );
    struct stat finfo;

    if(fstat(file_fd, &finfo) != 0) {
        perror("Fstat failed\n");
    }

    void *fs_img = mmap(NULL, finfo.st_size, MAP_PRIVATE, PROT_READ | PROT_WRITE, file_fd, 0);

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
        // tempreply = "test";
        tempreply = malloc(4);
        memcpy(tempreply, &inum, 4);
        // printf("%s\n", tempreply);
      }

      else if(msg->type == CREAT){
        printf(" message pinum: %d\n", msg->pinum);
        printf("Creating with name: %s\n", msg->name);

        int ret = Ser_MFS_Creat(fs_img);
        printf("Create done, it's %d\n", ret);
        // tempreply = "test";
        tempreply = malloc(4);
        memcpy(tempreply, &ret, 4);
      }

      // working after here
      if (rc > 0) {
        // do mmap flush or sync or something
        char reply[BUFFER_SIZE];
        sprintf(reply, "ok");
        msync(fs_img, finfo.st_size, PROT_READ | PROT_WRITE);
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