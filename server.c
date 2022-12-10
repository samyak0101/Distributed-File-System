#include <stdio.h>
#include "udp.h"

#define BUFFER_SIZE (100)

int sd;

void intHandler(int dummy) {
    UDP_Close(sd);
    exit(130);
}

// server code
int main(int argc, char *argv[]) {

    //OFFICIAL CODE, DO NOT DELETE:

    // assert(argc == 3);
    // int port_num = atoi(argv[1]);
    // const shar* img_path = argv[2];

    // sd = UDP_Open(port_num);
    // if(sd <= 0){
    //     //throw error
    //     perror("Failed to bind to UDP socket\n");
    //     return -1;
    // } else{
    //     printf("Listening for connections on port %i\n", port_num);
    // }

    // int file_fd = open(img_path, O_RDWR | O_APPEND);
    // struct stat finfo;

    // if(fstat(file_fd, &finfo) != 0) {
    //     perror("Fstat failed\n");
    // }

    // void *fs_img = mmap(NULL, finfo.st_size, MAP_SHARED, PROT_READ | PROT_WRITE, file_fd, 0);

    // super_t *superblock = superblock->inode_bitmap_len * sizeof(unsigned int)* 8;

    // END KAI'S CODE


    sd = UDP_Open(10000);
    assert(sd > -1);
    while (1) {
	struct sockaddr_in addr;
	char message[BUFFER_SIZE];
	printf("server:: waiting...\n");
	int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
	printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
	if (rc > 0) {
            char reply[BUFFER_SIZE];
            sprintf(reply, "server response:");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
	    printf("server:: reply\n");
	} 
    }

    signal(SIGINT, intHandler);

    return 0; 
}
    
    


 