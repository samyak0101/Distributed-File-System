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

#define BUFFER_SIZE (sizeof(messagestruct))

int serverfd;
messagestruct *msg;
super_t *superblock;
inode_t *inodes;
int max_inodes;
int file_fd;

//
void *read_bytes;
FileRead_t *fileread;  // Filled when reading a single
DirRead_t *dirread;    // Filled when reading directory
enum read_type readtype;
//

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
  // printf("position: %d\n", position);
   int index = position / 32;
   int offset = 31 - (position % 32);
   bitmap[index] |= (0x1 << offset);
}

void clear_bit(unsigned int *bitmap, int position) {
  // printf("position: %d\n", position);
   int index = position / 32;
   int offset = 31 - (position % 32);
   bitmap[index] &= ~(1UL << offset);
}

//helper
int alloc_databitmap(void* fs_img){
  // printf("inside alloc data bitmap \n");

  void *addr = fs_img + (superblock->data_bitmap_addr * MFS_BLOCK_SIZE);
  int valid;
  int position = -1;
  // TODO: change this loop to loop over entire data bitmap. Rn assuming only 1 block dbm.
  for (int i = 0; i < superblock -> num_data; i++){
    printf("calling getbit with i = %d\n", i);
    valid = get_bit((unsigned int *)addr, i);
    if(valid != 1){
      printf("valid was not 1\n");
      position = i;
      
      set_bit((unsigned int *)addr, i);
      printf("done setting bit!\n");
      break;
    }
  }
  // printf("returned value databitmap alloc: %d\n", position);

  return position;  
}

//helper
int alloc_inodebitmap(void* fs_img){
  printf("inside alloc inode bit\n");

  void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
  int valid;
  int position = -1;
  // TODO: change this loop to loop over entire data bitmap. Rn assuming only 1 block dbm.
  for (int i = 0; i < superblock -> num_inodes; i++){
    valid = get_bit((unsigned int *)addr, i);
    if(valid != 1){
      position = i;
      set_bit((unsigned int *)addr, i);
      break;
    }
  }
  // printf(" returned value inode bitmap alloc: %d\n", position);

  return position;  
}

// server MFS Stat
char* Ser_MFS_Stat(void* fs_img){
    // printf("MESSAGE TYPE! type: %d\n", msg->type);
    MFS_Stat_t *statstruct = &(msg->statstruct);

    // check if the inode is valid in inode bitmap
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->inum);

    // return struct with -1's if inode is invalid
    // printf("valid bit of root: %i\n", valid);

    if(valid != 1){
        statstruct->type = -1; 
        statstruct->size = -1; 
        return (char*)statstruct; 
    }

    printf("is it valid?: %d\n", valid);
    printf("what inum am i looking for? %d\n", msg->inum);

    // else, go to inode and read in inode struct 
    void *inode_offset = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE; 
    inode_offset = inode_offset +  sizeof(inode_t)*(msg->inum % 32); 
    
    // read in inode struct but as an MFS stat struct
    inode_t *inode = (inode_t*)inode_offset; 

    statstruct -> type = inode->type;
    statstruct -> size = inode->size;

    printf("Printing from stat!\n This is the size: %d\n This is the type: %d\n", inode->size, inode->type);
    
    // return stat struct 
    return (char*)statstruct;
}

// server MFS Lookup
int Ser_MFS_Lookup(void* fs_img){

    // TODO: fix method so it searches for string name in msg->name and return it's inum
    // Also do the error checking and retunrn appropriately when  the thing im looking for isn't there or something.
    // printf("MESSAGE TYPE! type: %d\n", msg->type);
    // printf("Looking for this name in lookup: %s\n", msg->name);

    // check if the inode is valid in inode bitmap
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->pinum);

    // return struct with -1's if inode is invalid
    // printf("valid bit of root: %i\n", valid);
    if(valid != 1){
        return -1;
    }

    // else, go to inode and read in inode struct
    void *inode_offset = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE;
    void *inode_offset2 = inode_offset;
    inode_offset = inode_offset +  (msg->pinum % 32)*sizeof(inode_t);
    printf("msg inum: %d\n", msg->pinum);
    printf("%p vs %p\n", inode_offset2, inode_offset);
    // read in inode struct but as an MFS stat struct

    inode_t *inode = (inode_t*)inode_offset;

    if(inode->type != 0){
      return -1;
    }

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
        // printf("name of dirent at this location: %s\n ", directory_entry->name);
        // check if the name is the same
        if(strcmp(msg->name, directory_entry->name)==0){
          found = 1;
          printf("Looked up inode %d -> %s\n", directory_entry->inum, msg->name);
          printf("Found this name in lookup: %s\n", directory_entry->name);
          printf("inum in lookup LOOKKKUPP: %d\n", directory_entry->inum);
          return directory_entry->inum;
        }
      }
    }
    return -1;
}

// server MFS Creat
int Ser_MFS_Creat(void* fs_img){ 


    // printf("Message Type: %d\n", msg->type);
    // check if the inode of the parent directory is valid
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->pinum);
    if(valid != 1){
      return -1; // pinum is invalid
    }
    // printf("pinum is valid\n");


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
    if(parentinode->type != 0){
      return -1;
    }

    int direct_ptr_datablock;
    void *iterator;
    // see if parent if parent directory is full or not, return -1 if full
    dir_ent_t *check;

    // int found_dir_space = -1;
    int found = -1;

    // find space for dir ent of new file/directory and save pointer to address space check.
    // printf("Looping through dirent to find free dir ent slot\n");
    for(int i = 0; i < DIRECT_PTRS; i++){
      // printf("parent inode dierct i is %d\n", parentinode->direct[i]);
      if(parentinode->direct[i] == -1){
        // printf("NOT SUPPOSED TO GO HERE! \n\n\n");
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
        // printf("checking for sub directories: \n");
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
    printf("Found space for inode, now need to alloc an inode to it\n");
    // file/directory does not exist, so allocate inode for it
    // printf("allocating inode to type %d\n", msg->ttype);
    int new_inode = alloc_inodebitmap(fs_img);
    if(new_inode < 0){
      printf("could not allocate a new inode that's the problem\n");
      return -1; // could not allocate since inode bitmap is full
    }
    printf("Allocated inode : %d\n", new_inode);
    // creating inode in inode region
    void *tempint = fs_img + superblock->inode_region_addr * MFS_BLOCK_SIZE;
  
    tempint += sizeof(inode_t) * new_inode;

    printf("here 10k idk\n");
    inode_t *new_inode_struct = (inode_t *)tempint;
    new_inode_struct->type = msg->ttype;

    for (int p = 0; p<30; p++){
      new_inode_struct->direct[p] = -1;
    }

    if(msg->ttype == 0){
      printf("director: %d\n", msg->ttype);
    }
    else{
      // printf("file: %d\n", msg->ttype);
    }
    if(msg->ttype==1){
      //almost done
      new_inode_struct -> size = 0;
      check->inum = new_inode;
      strcpy(check->name, msg->name);
      // printf("Created new file. Inum is %d \n", new_inode);
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
        // printf("checking for sub directories: \n");
        void *location = iterator + j*sizeof(dir_ent_t);
        check = (dir_ent_t *)location;
        check->inum = -1;
      }


    //write 2 dir ents to the new location
      void *inode_offset = fs_img + new_inode_struct->direct[0] * UFS_BLOCK_SIZE;
      dir_ent_t *newdir_dirent_one = (dir_ent_t *)inode_offset;
      
      // newdir_dirent_one->name = ".";
      // printf("Created new directory. Inum is %d and about to write children now:\n", new_inode);
      strcpy(newdir_dirent_one->name, ".");
      printf("new inode printed later: %d\n", new_inode);
      newdir_dirent_one->inum = new_inode;
      printf("Created directory '%s' inum '%d'\n", newdir_dirent_one->name, newdir_dirent_one->inum);

      newdir_dirent_one += 1;
      strcpy(newdir_dirent_one->name, "..");
      newdir_dirent_one->inum = msg->pinum;

      printf("Created directory '%s' inum '%d'\n", newdir_dirent_one->name, newdir_dirent_one->inum);

    return 0;
}

// server MFS Write - 
int Ser_MFS_Write(void* fs_img){
    printf("Inside Server MFS_Write; Message type: %d\n", msg->type);
    // Check if inum is valid, if not return -1
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->inum);
    if(valid != 1){
      return -1; // inum is invalid
    }
    // Check if nbytes is greater than 4096 or less than 0, if so return -1
    if(msg->nbytes > 4096 || msg->nbytes < 0){
      printf("2\n");
      return -1;
    }
    // Go to appropriate inode based on inum
    void *inode_off = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE;
    inode_off = inode_off + sizeof(inode_t)*(msg->inum % 32);
    // reading inode struct of parent.
    inode_t *inode = (inode_t*)inode_off;
    // Check if offset is greater than size of data block or less than 0, if so return -1
    if(msg->offset >= MFS_BLOCK_SIZE || msg->offset < 0){
      printf("3\n");
      return -1;
    }
    // Check i-bitmap and make sure bit corresponding to inum is allocated (set to 1)
    // void *tempptr = fs_img + superblock->inode_bitmap_addr * MFS_BLOCK_SIZE;
    // int bit = get_bit((unsigned int*)tempptr, msg->inum);
    // if(bit == 0){
    //   printf("4\n");
    //   return -1;
    // }
    // If i-bitmap is allocated at inum, check if file type is directory, if so return -1
    if(inode->type == MFS_DIRECTORY){
      printf("5\n");
      return -1;
    }

    printf("file size: %d\n", inode->size);
    // If inode's size is set to 0, try to allocate space for file in data region
    if(inode->size == 0){
      int datablock = alloc_databitmap(fs_img);
      if(datablock == -1){
        return -1;
      }
      inode->direct[0] = datablock + superblock->data_region_addr;      // helpful!!!
    }
    // Use fileinode->direct[0] and offset to reach byte to begin writing from
    void *tempptr = fs_img + (inode->direct[0] * MFS_BLOCK_SIZE) + msg->offset;
    // char *writeptr = (char*)tempptr;
    memcpy(tempptr, msg->buffer, msg->nbytes);
    // Update file's size in inode
    inode->size = msg->offset + msg->nbytes;
    return 0;
}

// Server MFS READ - Need to test
void *Ser_MFS_Read(void* fs_img){

    read_bytes = malloc(sizeof(msg->nbytes));
    char *negative_val = "-1";
    char *positive_val = "0";
    printf("Inside Server MFS_Read; Message type: %d\n", msg->type);
    // Check if inum is valid, if not return -1
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->inum);
    if(valid != 1){
      return (void *)negative_val;
    }
    // Go to appropriate inode based on inum
    void *inode_off = fs_img + (superblock->inode_region_addr + msg->inum/32) * MFS_BLOCK_SIZE;
    inode_off = inode_off + sizeof(inode_t)*(msg->inum % 32);
    // reading inode struct of parent.
    inode_t *inode = (inode_t*)inode_off;

    printf("size of file at inode: %d\n", inode->size);

    // Read behavior for file read
    if(inode->type == 1){
      // readtype is used in main for determining message to send back to client
      // readtype = FILEREAD;
      printf("reached here\n");
      // Check if offset is greater than size of file or less than 0, if so return -1
      if(msg -> offset > inode->size || msg->offset < 0){
      return (void *)negative_val;

      }

      // Check if nbytes is greater than size of file minus offset or less than 0, if so return -1
      if(msg->nbytes > inode->size || msg->nbytes < 0){
      return (void *)negative_val;

      }

      printf("congrats on reaching this part\n");
      if(inode->direct[0] < 0){
      return (void *)negative_val;

      }
      // Use fileinode->direct[0] and offset to reach byte to begin reading from
      void *tempptr = fs_img + (inode->direct[0] * MFS_BLOCK_SIZE) + msg->offset;
      // char *readptr = (char*)tempptr;
      
      int i = 0;
      printf("congrats on reaching this part 2\n");
      // for(i = 0; i < msg->nbytes; i++){
      //   read_bytes[i] = *readptr;
      //   readptr++;
      // }
      
      memcpy(read_bytes, tempptr, msg->nbytes);

      // if(i < MFS_BLOCK_SIZE){
      //   fileread->filedata[i] = '\0';
      // }
      printf("string read: %s\n", (char *)read_bytes);

      return read_bytes;

    }

    // Read behavior for directory read (fun)
    else{
      // If inode is a directory, offset and nbytes both must be multiples of 32
      if(inode->type == MFS_DIRECTORY && (msg->offset % 32 != 0 || msg->nbytes % 32 != 0)){
        return (void *)negative_val;
      }
        
      // readtype is used in main for determining message to send back to client
      // readtype = DIRREAD;

      // Check if nbytes is greater than 4096 or less than 0, if so return -1
      if(msg->nbytes > MFS_BLOCK_SIZE || msg->nbytes < 0){
        return (void *)negative_val;

      }
       
      // Get number of valid data block locations from inode's direct array
      int i = 0;
      int num = 0;
      while(inode->direct[i] >= superblock->data_region_addr && inode->direct[i] < (superblock->data_region_addr + superblock->num_data)){
        num++;
        i++;
      }
      // Check if offset is invalid, if so return -1
      if(msg->offset > num * MFS_BLOCK_SIZE || msg->offset < 0)
        return (void *)negative_val;

      // Go to directory entry specified by offset
      int index = msg->offset / MFS_BLOCK_SIZE;
      addr = fs_img + (inode->direct[index] * MFS_BLOCK_SIZE);
      addr = addr + (msg->offset % MFS_BLOCK_SIZE);
      dir_ent_t *currdirent = (dir_ent_t*)addr;
      int bytestoread = msg->nbytes;
      dirread = malloc(sizeof(DirRead_t));
      i = 0;
      int count = (msg->offset % MFS_BLOCK_SIZE) / sizeof(dir_ent_t);
      while(bytestoread > 0){
        dirread->direntries[i] = *currdirent;
        bytestoread - sizeof(DirRead_t);
        i++;
        count++;
        currdirent++;
        // if currdirent pointer points to end of a data block, set it to start of next data block
        if(count == 128){
          addr = fs_img + (inode->direct[index + 1] * MFS_BLOCK_SIZE);
          addr = addr + (msg->offset % MFS_BLOCK_SIZE);
          currdirent = (dir_ent_t*)addr;
        }
        bytestoread = bytestoread - sizeof(dir_ent_t);
      }
        return (void *)positive_val;
    }
}

// server MFS UNLINK
int Ser_MFS_Unlink(void* fs_img){


  printf("Trying to unlink a file called %s\n", msg->name);

  // check if pinum is valid
    printf("parent has the inum: %d\n", msg->pinum);
    void *addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    int valid = get_bit(addr, msg->pinum);

    // return struct with -1's if inode is invalid

    if(valid != 1){
      printf("parent inum aws invalid?\n");
        return -1;
    }

    printf("pinum is valid\n");

    // else, go to inode and read in inode struct
    void *inode_offset = fs_img + (superblock->inode_region_addr + msg->pinum/32) * MFS_BLOCK_SIZE;
    inode_offset = inode_offset +  sizeof(inode_t)*(msg->pinum % 32);
    // read in inode struct but as an MFS stat struct

    inode_t *parent_inode = (inode_t*)inode_offset;
    printf("Got the parent inode. It's size is: %d\n", parent_inode->size);

    int found = -1;
    int find_inum = -1;
    dir_ent_t *dirent_to_delete;


    for (int i = 0; i<DIRECT_PTRS; i++){

      if(parent_inode->direct[i] == -1){
        // return 0 because file name does not exist so maybe it has already been deleted
        printf("File name dne\n");
        return 0;
      }

      int location = parent_inode->direct[i] * UFS_BLOCK_SIZE;
      

      for (int j = 0; j < UFS_BLOCK_SIZE/sizeof(dir_ent_t); j++){
        printf("entered second for loop\n");
        void *dir_ent_offset = fs_img + location + sizeof(dir_ent_t)*j;
        dirent_to_delete = (dir_ent_t *)dir_ent_offset;
        // printf("name of dirent at this location: %s\n ", directory_entry->name);
        // check if the name is the same
        if(strcmp(msg->name, dirent_to_delete->name)==0){
          printf("Found dir ent with name %s in parent's directory entries\n", msg->name);
          found = 1;
          find_inum = dirent_to_delete->inum;
          break;
        }
      }
      if(found == 1){
        break;
      }
    }

    if(found == -1){
      printf("Not Supposed to be here!\n");
      return 0;
    }
    if(find_inum == -1){
      printf("NOT SUPPOSED TO BE HERE!!! \n\n");
    }

    inode_offset = fs_img + (superblock->inode_region_addr + find_inum/32) * MFS_BLOCK_SIZE;
    inode_offset = inode_offset + sizeof(inode_t) * (find_inum % 32);
    // read in inode struct but as an MFS stat struct

    inode_t *inode_to_delete = (inode_t*)inode_offset;
    printf("inode to delete size: %d\n inode to del   type: %d\n", inode_to_delete->size, inode_to_delete->type);


    if(inode_to_delete->type == 0 && inode_to_delete->size > 64){
      return -1; // cannot delete non-empty directory.
    }

    printf("Reached, about to clear inode and some other stuff\n");
    // clear inode bitmap bit
    addr = fs_img + (superblock->inode_bitmap_addr * MFS_BLOCK_SIZE);
    printf("attempting to clear %d\n", find_inum);
    clear_bit((unsigned int *)addr, find_inum);

    
    int del_dbit = inode_to_delete->direct[0];
    printf("Got past clear_bit i think!\n");
    printf("value of delbit is %d\n", del_dbit);

    if(del_dbit >= 0){
      // clear dbit
      addr = fs_img + (superblock->data_bitmap_addr * MFS_BLOCK_SIZE);
      printf("before second clear bit\n");
      clear_bit((unsigned int *)addr, del_dbit);
      printf("after second clear bit \n");
      inode_to_delete->direct[0] = -1;
    }

    printf("Last 2 things to change!\n");
    dirent_to_delete->inum = -1;
    parent_inode->size -= 32;
      
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

    file_fd = open(img_path, O_RDWR);
    struct stat finfo;

    if(fstat(file_fd, &finfo) != 0) {
        perror("Fstat failed\n");
    }

    void *fs_img = mmap(NULL, finfo.st_size, MAP_PRIVATE, PROT_READ | PROT_WRITE, file_fd, 0);

    superblock = (super_t*)fs_img;
    max_inodes = superblock->num_inodes;

    inodes = fs_img + superblock->inode_region_addr * UFS_BLOCK_SIZE;

    printf("inodes %d\n", inodes->type);
    
    printf("max inum: %i\n", max_inodes);
// delete these
    // fprintf(stderr, "max inodes: %d\n", superblock->num_inodes);
    // exit(0);
// delete until here

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
        // printf("lookup message inum: %d\n", msg->inum);
        printf("lookup message pinum: %d\n", msg->pinum);
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

      else if(msg->type == READ){
        printf("calling read\n");
        Ser_MFS_Read(fs_img);
        printf("jere?\n");
        tempreply = read_bytes;
        printf("jere?\n");
        // memcpy(tempreply, read_bytes, msg->nbytes);
        // free(read_bytes);

        // if(readtype == FILEREAD){
        //   strcpy(tempreply, fileread->filedata);
        //   free(fileread);
        // }
        // else{
        //   strcpy(tempreply, (char*)dirread);
        //   free(dirread);
        // }
      }

      else if(msg->type == WRITE){
        printf("calling write\n");
        int ret = Ser_MFS_Write(fs_img);
        printf("Write done, it's %d\n", ret);
        // tempreply = "test";
        tempreply = malloc(4);
        memcpy(tempreply, &ret, 4);
      }

      else if(msg->type == SHUTDOWN){
        printf("Calling shutdown\n");
        msync(fs_img, finfo.st_size, PROT_READ | PROT_WRITE);
        UDP_Close(serverfd);
        close(file_fd);
        exit(0);
      }

      else if(msg->type == UNLINK){
        printf("Calling unlink\n");
        int inum = Ser_MFS_Unlink(fs_img);
        printf("Unlink done, it's %d\n", inum);
        tempreply = malloc(4);
        memcpy(tempreply, &inum, 4);
      }

      // working after here
      if (rc > 0) {
        // do mmap flush or sync or something
        msync(fs_img, finfo.st_size, PROT_READ | PROT_WRITE);
        rc = UDP_Write(serverfd, &addr, tempreply, sizeof(messagestruct));
        printf("server:: reply\n");
      } 
    }

    signal(SIGINT, intHandler);

    return 0; 
}


