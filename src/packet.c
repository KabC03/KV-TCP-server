#include "./packet.h"

const char *OPERATION_KEYWORDS[] = {
    "connect",
    "insert",
    "delete",
    "lookup",
    "rehash",
    "terminate",
};


/* internal_str_to_size_t */
/* Convert a string to a valid size_t */
bool internal_str_to_size_t(const char *string, size_t *out) {
    if(!string || !out) {
        return false;
    }

    char *end = NULL;
    errno = 0;

    unsigned long long val = strtoull(string, &end, 10);
    if(errno != 0 || *end != '\0' || string == end) {
        return false;
    }

    *out = (size_t)val;
    return true;
}


/* clean_input */
/* Remove newline chars from user input */
void clean_input(char *string) {
    if(!string) {
        return;
    }

    while(*string != '\0') {
        if(*string == '\n' || *string == '\r') {
            *string = '\0';
            return;
        }
        string++;
    }
}


/* send_all */
/* Keep sending until all bytes are gone */
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


/* recv_all */
/* Keep receiving until the whole buffer is filled */
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


/* create_packet */
/* Create a packet from a corrosponding string, handling all errors */
bool create_packet(char *input, Packet *packetOut) {
    if(!input || !packetOut) {
        printf("\tEmpty client request\n");
        return false;
    }

    clean_input(input);

    size_t inputLength = strlen(input);
    if(inputLength == 0) {
        printf("\tEmpty client request\n");
        return false;
    }

    if(inputLength > MAX_INPUT_SIZE) {
        printf("\tMaximum input size (%d chars) exceeded\n", MAX_INPUT_SIZE);
        return false;
    }

    //Clear old packet contents first
    memset(packetOut, 0, sizeof(Packet));

    char *operation = strtok(input, " ");
    if(!operation) {
        printf("\tMissing operation\n");
        return false;
    }

    char *field1 = strtok(NULL, " ");
    char *field2 = strtok(NULL, " ");
    char *field3 = strtok(NULL, " ");

    //Connect - establish connection to server
    if(strcmp(OPERATION_KEYWORDS[0], operation) == 0) {
        packetOut->metadata.op = CONNECT;

        if(!field1) {
            printf("\tExpect IP address field for connection\n");
            return false;
        }

        packetOut->data.field1 = field1;
        packetOut->metadata.field1Length = strlen(field1) + 1; //Include \0

    //Insert - add key-value pair
    } else if(strcmp(OPERATION_KEYWORDS[1], operation) == 0) {
        packetOut->metadata.op = INSERT;

        if(!field1) {
            printf("\tExpect key field\n");
            return false;
        }
        if(!field2) {
            printf("\tExpect value field\n");
            return false;
        }

        packetOut->data.field1 = field1;
        packetOut->metadata.field1Length = strlen(field1) + 1;

        packetOut->data.field2 = field2;
        packetOut->metadata.field2Length = strlen(field2) + 1;

    //Delete - remove key from server
    } else if(strcmp(OPERATION_KEYWORDS[2], operation) == 0) {
        packetOut->metadata.op = DELETE;

        if(!field1) {
            printf("\tExpect key field\n");
            return false;
        }

        packetOut->data.field1 = field1;
        packetOut->metadata.field1Length = strlen(field1) + 1;

    //Lookup - retrieve value for key
    } else if(strcmp(OPERATION_KEYWORDS[3], operation) == 0) {
        packetOut->metadata.op = LOOKUP;

        if(!field1) {
            printf("\tExpect key field\n");
            return false;
        }

        packetOut->data.field1 = field1;
        packetOut->metadata.field1Length = strlen(field1) + 1;

    //Rehash - resize hash table
    } else if(strcmp(OPERATION_KEYWORDS[4], operation) == 0) {
        packetOut->metadata.op = REHASH;

        if(!field1) {
            printf("\tExpect new map size\n");
            return false;
        }

        if(!internal_str_to_size_t(field1, &(packetOut->data.field3))) {
            printf("\tInvalid map size passed\n");
            return false;
        }

    //Terminate - end session
    } else if(strcmp(OPERATION_KEYWORDS[5], operation) == 0) {
        packetOut->metadata.op = TERMINATE;

    } else {
        printf("\tUnrecognised operation: '%s'\n", operation);
        return false;
    }

    //Extra fields only allowed for insert
    if(field3 && packetOut->metadata.op != INSERT) {
        printf("\tToo many fields provided\n");
        return false;
    }

    return true;
}


/* process_packet */
/* Process a packet request from a client */
bool process_packet(Packet *packet, char *response) {
    static bool connected = false;
    static int sockfd = -1; //Only one active server for now

    if(!packet) {
        return false;
    }

    //Must be connected before any operation except CONNECT
    if(!connected && packet->metadata.op != CONNECT) {
        printf("Not connected to server\n");
        return false;
    }

    //Cannot connect to new server while already connected
    if(connected && packet->metadata.op == CONNECT) {
        printf("Already connected to a server. Terminate current connection first\n");
        return false;
    }

    switch(packet->metadata.op) {
        case CONNECT: {
            printf("\tAttempting connection to '%s'\n", packet->data.field1);

            //Create socket
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                printf("Socket establishment error: %s\n", strerror(errno));
                return false;
            }

            //Setup server address
            struct sockaddr_in serverAddr;
            memset(&serverAddr, 0, sizeof(serverAddr));

            
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(PORT);

            //Convert IP string to binary
            if(inet_pton(AF_INET, packet->data.field1, &(serverAddr.sin_addr)) <= 0) {
                printf("inet_pton error: %s\n", strerror(errno));
                close(sockfd);
                sockfd = -1;
                return false;
            }

            //Attempt connection
            if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
                printf("Connection error: %s\n", strerror(errno));
                close(sockfd);
                sockfd = -1;
                return false;
            }

            connected = true;
            printf("Connected\n");
            return true;
        }

        case INSERT:
        case DELETE:
        case LOOKUP:
        case REHASH:
        case TERMINATE: {
            //Send metadata first so the server knows what is coming
            if(!send_all(sockfd, &(packet->metadata), sizeof(packet->metadata))) {
                printf("Error sending packet metadata to server\n");
                return false;
            }

            //Send field1 if present
            if(packet->metadata.field1Length > 0 && packet->data.field1) {
                if(!send_all(sockfd, packet->data.field1, packet->metadata.field1Length)) {
                    printf("Error sending field1 to server\n");
                    return false;
                }
            }

            //Send field2 if present
            if(packet->metadata.field2Length > 0 && packet->data.field2) {
                if(!send_all(sockfd, packet->data.field2, packet->metadata.field2Length)) {
                    printf("Error sending field2 to server\n");
                    return false;
                }
            }

            //Send field3 (size) for rehash
            if(packet->metadata.op == REHASH) {
                if(!send_all(sockfd, &(packet->data.field3), sizeof(packet->data.field3))) {
                    printf("Error sending field3 to server\n");
                    return false;
                }
            }

            //Only lookup expects an actual string response
            if(packet->metadata.op == LOOKUP) {
                size_t responseLength = 0;

                if(!recv_all(sockfd, &responseLength, sizeof(responseLength))) {
                    printf("Error receiving response length from server\n");
                    return false;
                }

                if(responseLength == 0 || responseLength > MAX_INPUT_SIZE) {
                    printf("Invalid response length from server\n");
                    return false;
                }

                if(!response) {
                    printf("Client response buffer is NULL\n");
                    return false;
                }

                if(!recv_all(sockfd, response, responseLength)) {
                    printf("Error receiving lookup response from server\n");
                    return false;
                }
            }

            //Clean up on terminate
            if(packet->metadata.op == TERMINATE) {
                close(sockfd);
                sockfd = -1;
                connected = false;
                printf("Disconnected\n");
            }

            return true;
        }
    }

    return false;
}



