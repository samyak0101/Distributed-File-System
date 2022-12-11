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
    - 

Questions:
    - if root dir starts from 0 then how many inodes per block? if inum is 5 then where do  get it. 



    Client randomly picks its own port number within mfs.c
    mfs init uses the destination port number to connect to the server
    server gets message from client and so fills in the source socket address from client portnumber
    

gcc -o client mfs.c udp.c client.c