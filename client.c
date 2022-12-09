#include <stdio.h>
#include "udp.h"

#define BUFFER_SIZE (1000)

static int destination_port = 54897;

// client code
int main(int argc, char *argv[]) {

    struct sockaddr_in addrSnd, addrRcv;

    int rc = MFS_Init("localhost", destination_port);


    char message[BUFFER_SIZE];
    sprintf(message, "hello world");

    printf("client:: send message [%s]\n", message);
    rc = MFS_Write(serverport, message, 0, BUFFER_SIZE);
    if (rc < 0) {
	printf("client:: failed to send\n");
	exit(1);
    }

    printf("client:: wait for reply...\n");
    rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
    printf("client:: got reply [size:%d contents:(%s)\n", rc, message);
    return 0;
}
