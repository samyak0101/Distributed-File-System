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
    - 


    Client randomly picks its own port number within mfs.c
    mfs init uses the destination port number to connect to the server
    server gets message from client and so fills in the source socket address from client portnumber
    

gcc -o out mfs.c udp.c client.c