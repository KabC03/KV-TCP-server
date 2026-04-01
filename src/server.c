#include "packet.h"
#include "hashmap.h"

static bool send_all(int sockfd, const void *buffer, size_t length) {
    const char *ptr = (const char *)buffer;
    size_t totalSent = 0;

    while(totalSent < length) {
        ssize_t sent = send(sockfd, ptr + totalSent, length - totalSent, 0);
        if(sent <= 0) {
            return false;
        }
        totalSent += (size_t)sent;
    }

    return true;
}

static bool recv_all(int sockfd, void *buffer, size_t length) {
    char *ptr = (char *)buffer;
    size_t totalRecv = 0;

    while(totalRecv < length) {
        ssize_t recvd = recv(sockfd, ptr + totalRecv, length - totalRecv, 0);
        if(recvd <= 0) {
            return false;
        }
        totalRecv += (size_t)recvd;
    }

    return true;
}

static bool handle_client(int clientfd, Hashmap *map) {
    while(true) {
        Packet packet;
        memset(&packet, 0, sizeof(Packet));

        //Receive packet metadata
        if(!recv_all(clientfd, &(packet.metadata), sizeof(packet.metadata))) {
            printf("Client disconnected or metadata receive failed\n");
            return false;
        }

        //Receive field1 if present
        if(packet.metadata.field1Length > 0) {
            packet.data.field1 = malloc(packet.metadata.field1Length);
            if(!packet.data.field1) {
                printf("Malloc failed for field1\n");
                return false;
            }

            if(!recv_all(clientfd, packet.data.field1, packet.metadata.field1Length)) {
                printf("Receive failed for field1\n");
                free(packet.data.field1);
                return false;
            }
        }

        //Receive field2 if present
        if(packet.metadata.field2Length > 0) {
            packet.data.field2 = malloc(packet.metadata.field2Length);
            if(!packet.data.field2) {
                printf("Malloc failed for field2\n");
                free(packet.data.field1);
                return false;
            }

            if(!recv_all(clientfd, packet.data.field2, packet.metadata.field2Length)) {
                printf("Receive failed for field2\n");
                free(packet.data.field1);
                free(packet.data.field2);
                return false;
            }
        }

        //Receive field3 for rehash
        if(packet.metadata.op == REHASH) {
            if(!recv_all(clientfd, &(packet.data.field3), sizeof(packet.data.field3))) {
                printf("Receive failed for field3\n");
                free(packet.data.field1);
                free(packet.data.field2);
                return false;
            }
        }

        switch(packet.metadata.op) {
            case INSERT: {
                const char *existing = hashmap_find(map, packet.data.field1, packet.metadata.field1Length);

                if(existing) {
                    if(hashmap_edit(map, packet.data.field1, packet.metadata.field1Length,
                                    packet.data.field2, packet.metadata.field2Length) == NULL) {
                        printf("Edit failed\n");
                    } else {
                        printf("Edited key '%s'\n", packet.data.field1);
                    }
                } else {
                    if(hashmap_insert(map, packet.data.field1, packet.metadata.field1Length,
                                      packet.data.field2, packet.metadata.field2Length) == NULL) {
                        printf("Insert failed\n");
                    } else {
                        printf("Inserted key '%s'\n", packet.data.field1);
                    }
                }
                break;
            }

            case DELETE: {
                if(hashmap_delete(map, packet.data.field1, packet.metadata.field1Length)) {
                    printf("Deleted key '%s'\n", packet.data.field1);
                } else {
                    printf("Key '%s' not found\n", packet.data.field1);
                }
                break;
            }

            case LOOKUP: {
                const char *value = hashmap_find(map, packet.data.field1, packet.metadata.field1Length);

                if(!value) {
                    char notFound[] = "NOT FOUND";
                    size_t len = strlen(notFound) + 1;

                    if(!send_all(clientfd, &len, sizeof(len))) {
                        free(packet.data.field1);
                        free(packet.data.field2);
                        return false;
                    }
                    if(!send_all(clientfd, notFound, len)) {
                        free(packet.data.field1);
                        free(packet.data.field2);
                        return false;
                    }

                    printf("Lookup key '%s' -> NOT FOUND\n", packet.data.field1);
                } else {
                    size_t len = strlen(value) + 1;

                    if(!send_all(clientfd, &len, sizeof(len))) {
                        free(packet.data.field1);
                        free(packet.data.field2);
                        return false;
                    }
                    if(!send_all(clientfd, value, len)) {
                        free(packet.data.field1);
                        free(packet.data.field2);
                        return false;
                    }

                    printf("Lookup key '%s' -> '%s'\n", packet.data.field1, value);
                }
                break;
            }

            case REHASH: {
                if(hashmap_rehash(map, packet.data.field3, NULL)) {
                    printf("Rehashed map to %zu buckets\n", packet.data.field3);
                } else {
                    printf("Rehash failed\n");
                }
                break;
            }

            case TERMINATE: {
                printf("Client requested termination\n");
                free(packet.data.field1);
                free(packet.data.field2);
                return true;
            }

            case CONNECT: {
                //Client should not send CONNECT to server after TCP connect
                printf("Ignoring CONNECT packet on server\n");
                break;
            }
        }

        free(packet.data.field1);
        free(packet.data.field2);
    }

    return true;
}

int main(void) {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd < 0) {
        printf("Socket creation failed: %s\n", strerror(errno));
        return 1;
    }

    int opt = 1;
    if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("setsockopt failed: %s\n", strerror(errno));
        close(serverfd);
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if(bind(serverfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Bind failed: %s\n", strerror(errno));
        close(serverfd);
        return 1;
    }

    if(listen(serverfd, 5) < 0) {
        printf("Listen failed: %s\n", strerror(errno));
        close(serverfd);
        return 1;
    }

    Hashmap map;
    if(!hashmap_init(&map, 16, hashmap_djb2)) {
        printf("Hashmap init failed\n");
        close(serverfd);
        return 1;
    }

    printf("Server listening on port %d\n", PORT);

    while(true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientfd = accept(serverfd, (struct sockaddr *)&clientAddr, &clientLen);
        if(clientfd < 0) {
            printf("Accept failed: %s\n", strerror(errno));
            continue;
        }

        printf("Client connected\n");

        handle_client(clientfd, &map);

        close(clientfd);
        printf("Client closed\n");
    }

    hashmap_destroy(&map);
    close(serverfd);
    return 0;
}



