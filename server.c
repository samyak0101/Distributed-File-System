#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (100)

// server code
int main(int argc, char *argv[]) {
    if(argc != 3)
        return -1;
    int portnum = argv[1];
    char *fsimage = argv[2];

    int sd = UDP_Open(portnum);
    assert(sd > -1);
    while (1) {
	struct sockaddr_in addr;
	char message[BUFFER_SIZE];
	printf("server:: waiting...\n");
	int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
	printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
	if (rc > 0) {
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye world");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
	    printf("server:: reply\n");
	} 
    }
    return 0; 
}

mfsinit()

    


 