# osp4
Fall 2022 OS p4: Building a Distributed File System 

TODO:
    - Add timeout functionality to udp (ask peer mentor how to add and for how long)
        - question: how many timeouts etc to add ?
    - Make mfs stat method work
    - 

NOTES: 
    - Use ss -ulpn to check port numbers and file descriptor of open ports
    - we are reading the whole thing into memory using mmap and flushing it to disk with msync
    - Use ./mkfs -ffs.img -d32 -i64 -v to ceck the layout of the file system image (need to compile mkfs before doing this obviously)
    - Used gcc -o out mfs.c udp.c client.c for compiling the client
    - 

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
    - 

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
    - After finding free block, go to that address in data region and save that places address.
    - 5: loop through inode bitmap to find free bit. If exist, assign it. If not exist, return -1 since no space. (Make sure to loop through all inode bitmap blocks).
    - go to inode region and create a new struct of inode
    - write type and size of the creation
    - if type is file, then go to that saved address and create a new dirent with filename and inum of file
    - if type is directory then make new dir ent in saved place with new directry name and inum.
    - find new bit in data bitmap and allocate to the first dirent entry of new dirctory inode. If not found, return -1.
    - go to that block and create 2 new dir entries for . and ..

Write Notes:
    - Information read by server: inum of file to write, bytes to actually write (char *buffer),
      offset into file's data block in bytes, and the number of bytes in the buffer
    - Check if inum is less than 0 or greater than max_inodes, if so, return -1
    - If nbytes is greater than 4096, less than zero, or greater than (4096 - offset), return -1
    - Get inode based on inum
    - If offset is greater than size (in inode) or less than zero, return -1
    - Check i-bitmap and make sure bit corresponding to inum is allocated (set to 1)
    - If i-bitmap is 1 for inumth bit, check if corresponding inode's type is regular file or directory; if i-bitmap is 0 for inumth bit, return -1
    - If inode's type is directory, return -1 (can't write directories)
    - If inode's size is set to 0, try to allocate space for file in data region
    - If all bits in d-bitmap are set to 1, no space to allocate, return -1
    - Otherwise, change first 0 in d-bitmap to 1 and save corresponding index to inode's direct[0]
    - If inode's size is greater than 0, just look at direct[0]
    - Use char* pointer and point to byte specified by offset in allocated data block
    - Use for loop to increment pointer and write buffer into file
    - Update file's size in inode
    - return 0

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
    - Use for loop to increment pointer and write from file into buffer
    - Manually null terminate buffer
    - return 0

Unlink Notes:
    - Set directory entry inum in parent inode where name matches name passed by client to -1
    - Get inum of directory entry and go to corresponding inode
    - I did a shit ton and rn I'm just too lazy to take all the notes; I made good comments tho

Questions:
    - if data full do we allocate file inode still (cuz it can't have data)
    - how to do mmap sync or flush etc.
    - in creat sometimes there are random garbage values at the dir ent location, how do i know for sure if theres a dir entry at a specific location or not?
    


    Client randomly picks its own port number within mfs.c
    mfs init uses the destination port number to connect to the server
    server gets message from client and so fills in the source socket address from client portnumber
    

gcc -o client mfs.c udp.c client.c
server 52364 file

Tests we have passing:
    - build
    - shutdown
    - creat
    - dir1
    - dir2
    - name
    - bigdir
    - deep

Tests we need to pass still:
    - write
    - stat
    - overwrite
    - maxfile
    - maxfile2
    - baddir
    - baddir2
    - unlink
    - unlink2
    - empty
    - persist