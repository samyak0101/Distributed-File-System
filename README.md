# osp4
Fall 2022 OS p4: Building a Distributed File System 


NOTES: 
    - Use ss -ulpn to check port numbers and file descriptor of open ports
    - we are reading the whole thing into memory using mmap and flushing it to disk with msync
    - Use ./mkfs -ffs.img -d32 -i32 -v to check the layout of the file system image (need to compile mkfs before doing this obviously)
    - Used gcc -o out mfs.c udp.c client.c for compiling the client
    - in unlink set inode to -1.

Client:
    - Open client socket
    - Fill udp message
    - Write udp message to target socket
    - Read socket response (wait implicitly)
    - Print server response

Server:
    - Open server socket (different port from client socket)
    - Read socket message (read implicitly waits)
    - Write response to client
    - Print client message

Super block:
    - info about file system and interacting w OS

Inodes:
    - 32 byte data structure
    - 16 inodes
    - Metadata about the file system
    - Can store 8 or 16 (not sure) blocks of data (512 bytes each), so 4096 per file.
    - Find the file size, then you know how many blocks to traverse
   
Specifications:
    - The file system image can be customized to include 128 or more inodes, and 32 or more data blocks
    - Each file can have up to 30 blocks of data allocated to it
    - Each directory will mostly only have 1 block of data allocated to it (but it can have 30).
    - The set_bit, get_bit, clear_bit functions are the most important to get correct for the system to work.
    - Unlink checks if directory size is more than 64, then it fails as it cannot unlink a non empty dir.
    - By default, files should have size  0 after creating, and directories should have size 64 (. and ..)
   
   
File contents
    - file payloads stored in quantums of block size
    - each file block has an inode number and offset
    - The whole file system is made up of blocks. We want to make most of these data storage blocks
    - Other blocks will store metadata about the file system. This metadata is stored is structs called information nodes.
    - The inode table holds an array of on-disk inodes. 
    - If there are 64 inodes in a file system, there will be 2 Inode bloks since each block     an only hold 32 inodes (if inodes are 128 bytes).
    - Each block is 4K: 4096 bytes in length 
    - We need to store information about whether a data block or inode block is allocated or not. 
    - For this, we use inode bitmap and data bitmap. 
    - So, we use a 4k block to store the inode bitmap which will tell us whether inode number 35 for example, is allocated or not. 
    - If it is, then we skip three 4k blocks since we skip super block, inode bitmap, data bitmap, to reach the start of the inode table (12KB from start)
    - Then, if we want the 32nd inode, we go to byte number 12KB + (128 * (inode_num - 1)) = 16256
    - Inode contains information about the type of file (f or d), the size, etc.
    - Each directory has 2 entries at least: the current director ( . ) and the parent directory ( .. ). 
    - Each dirctory or file name can only be 28 char string in length at max.
    - Each file and directory have their own inode. Each inode has an array of dir_entries. Each of these enties points to a different block in memory
    - There can further be 128 other dir_ent structs at these blocks in memory


Creat notes
- Another edgecase: Need to check if file or dir already exists. If it does, return -1 or someting before creating new space for anything.
    - Creating either file or directory under a parent inum of name *name;
    - Check if pinum exists and is valid. If not, return -1 since parent does not exist
    - Loop through parent inode direntries blocks to find free space for another dir ent. If can't find space, make space. If can't make space, -1.
    - If space found, continue to step 5, else:
    - Loop through data bitmap to find free block. When found, assign that int to the next free dirent index of parent
    - change size of parent to increase by sizeof directory entry.
    - After finding free block, go to that address in data region and save that places address.
    - 5: loop through inode bitmap to find free bit. If exist, assign it. If not exist, return -1 since no space. (Make sure to loop through all inode bitmap blocks).
    - go to inode region and create a new struct of inode
    - Loop over all direct[i] of inode and set them all to -1. (for file and dir both).
    - write type and size of the creation
    - if type is file, then go to that saved address and create a new dirent with filename and inum of file
    - if type is directory then make new dir ent in saved place with new directry name and inum.
    - find new bit in data bitmap and allocate to the first dirent entry of new dirctory inode. If not found, return -1.
    - go to that block, and create 128 dir ents with inum = -1.
    - Then go to the start of the same block and make 2 directory entries: . and ..
    - Make sure you have updated size of new dir and parent dir accordingly (parent dir increments by 32 for file and dir).

Write Notes:
    - Information read by server: inum of file to write, bytes to actually write (char buffer), offset into file's data block in bytes, and the number of bytes in the buffer 
    - Check if inum is less than 0 or greater than max_inodes, if so, return -1 - If nbytes is greater than 4096, less than zero, or greater than (4096 - offset), return -1 
    - Get inode based on inum - If offset is greater than size (in inode) or less than zero, return -1 - Check i-bitmap and make sure bit corresponding to inum is allocated (set to 1) 
    - If i-bitmap is 1 for inumth bit, check if corresponding inode's type is regular file or directory; if i-bitmap is 0 for inumth bit, return -1 
    - If inode's type is directory, return -1 (can't write directories) 
    - If inode's size is set to 0, try to allocate space for file in data region 
    - If all bits in d-bitmap are set to 1, no space to allocate, return -1 
    - Otherwise, change first 0 in d-bitmap to 1 and save corresponding index to inode's direct[0] 
    - If inode's size is greater than 0, just look at direct[0] 
    - Use char pointer and point to byte specified by offset in allocated data block - Use for loop to increment pointer and write buffer into file 
    - Update file's size in inode - return 0

Read Notes: 
    - Information read by server is the same as for Write 
    - Check if inum is less than 0 or greater than max_inodes, if so, return -1 
    - Get inode based on inum 
    - If inode->type is directory, nbytes and offset must both be multiples of 32 
    - If inode->type is regular file: 
    - offset must be between 0 and inode->size 
    - nbytes must be between 0 and inode->size - offset 
    - Create _read struct so bytes read can be sent back to client 
    - Use char* pointer and point to byte specified by offset in allocated data block 
    - Use for loop to increment pointer and write from file into buffer - Manually null terminate buffer 
    - return 0
    
Questions:
    - if data full do we allocate file inode still (cuz it can't have data)
    - how to do mmap sync or flush etc.
    - in creat sometimes there are random garbage values at the dir ent location, how do i know for sure if theres a dir entry at a specific location or not?
    
Unlink notes:
    - removes file or dir specified by pinum; ret 0 on success -1 on failure
    - Check if pinum is valid. If not, return -1.
    - Loop through parent's directory entries looking for name
    - If found name, store inum (file and dir both), check dir ent. Get inum and check if file. 
    - if file, clear inode bitmap 
    - check direct[0]. If alloc'd, set to -1
    - Go to data bitmap and set bit to 0.
    - if directory, check size. If size > 64, return -1 since dir is not empty. if size == 64, then clear inode bitmap, clear dirent[0] and data bitmap
    - reduce parent size field and set that dirent's inum to -1
    - remove inode corresponding to directory


gcc -o client mfs.c udp.c client.c
server 52364 file
