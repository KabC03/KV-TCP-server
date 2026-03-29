#include "../packet.h"


int main(void) {


    int serverFd;
    int clientFd;

    serverFd = (AF_INET, SOCK_STREAM, 0);
    if(serverFd < 0) {
        printf("Socket establishment error\n");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverFd, (struct sockaddr*)&addr, sizeof(addr) < 0)) {
        printf("Bind error\n");
        close(serverFd);
        return 1;
    }

    if(listen(serverFd, 10) < 0) {
        printf("Listen error\n");
        return 1;
    }


    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);


    clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);

    printf("Client connected\n");



    return 0;
}


